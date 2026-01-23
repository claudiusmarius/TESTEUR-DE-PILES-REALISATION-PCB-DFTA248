// Testeur de piles AA et AAA
// Voir vidéo YouTube https://youtu.be/V9YFY4Gzfeo
// Voir GitHub https://github.com/claudiusmarius/TESTEUR-DE-PILES-REALISATION-PCB-DFTA248
// Auteur Claude DUFOURMONT


#include <Adafruit_NeoPixel.h>

/*
====================================================================
                        SCHÉMA DES BARRETTES
====================================================================

────────────────────────────────────────────────────────
 BARRETTE 1 — Tension à vide
────────────────────────────────────────────────────────
Index :   0     1     2     -     3

LED   :  Rouge Orange Vert  -   Stable (V/B)
→ Évalue uniquement la tension à vide, donc la "capacité restante brute".

────────────────────────────────────────────────────────
 BARRETTE 2 — Tension en charge
────────────────────────────────────────────────────────
Index :   4     5     6    -    7
LED   :  Rouge Orange Vert  -  Stable (V/B)
→ Test plus réaliste : révèle la chute interne et l’état réel de la pile.

────────────────────────────────────────────────────────
 BARRETTE 3 — Diagnostic final ΔV + cohérence
────────────────────────────────────────────────────────
Index :   8    9    10    -     11
LED   :  Rouge Orange Vert  -  Alarme (R/B)

11 bleu  = surtension
11 R/B clignotant = inversion ou Vbat < 50 mV
11 rouge fixe = fin alarme surtension

────────────────────────────────────────────────────────
 NOTE IMPORTANTE :
 BP1 = RESET HARDWARE via PB5.
 → Aucun traitement logiciel dans ce code.
────────────────────────────────────────────────────────
*/


// ------------------------------------------------------------
//  BROCHAGE PHYSIQUE DE L’ATTINY85
// ------------------------------------------------------------
#define PIN_LED     0      // Sortie vers les WS2812B
#define PIN_CHARGE  1      // Commande du MOSFET de charge
#define PIN_VBAT    A1     // Lecture tension pile
#define PIN_RSENSE  A3     // Lecture tension sur résistance Rsense
#define PIN_BUZZER  4      // Petit buzzer PIÉZO

//#define N_PIXELS 24        // Nombre total de LED (3 rangées de 8)
#define N_PIXELS 12        // Nombre total de LED (3 rangées de 4)
#define RSENSE   2.2f      // Résistance de mesure de courant CORRESPOND A LA RESISTANCE R4


Adafruit_NeoPixel strip(N_PIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);


// ------------------------------------------------------------
//  CALIBRATIONS (à ajuster si besoin)
// ------------------------------------------------------------
const float VREF_3V3 = 3.307f;                                  // Tension réelle du régulateur 3.3V
const float VBAT_multimetre = 1.4970f;                          // Mesure réelle de la pile au multimètre
const float VDIV_multimetre = 1.4970f;                          // Idem car pas de pont diviseur
float K_DIV = VDIV_multimetre / VBAT_multimetre;                // Toujours = 1 ici


// ------------------------------------------------------------
//  SEUILS DE CLASSEMENT PILES
// ------------------------------------------------------------
float seuil_TresBas_vide     = 1.10;
float seuil_Acceptable_vide  = 1.36;
float seuil_TresBas_charge    = 1.10;
float seuil_Acceptable_charge = 1.36;


// ======================================================================
//           ROUTINE DE LECTURE ADC AMÉLIORÉE (stabilisation)
// ======================================================================
uint16_t readADC_stable(uint8_t pin) {
  analogRead(pin);                                                // 1ère lecture pour amorcer le S/H
  delayMicroseconds(250);                                         // petit délai pour stabiliser la capa interne
  return analogRead(pin);                                         // On retourne la 2ème lecture, beaucoup plus fiable
}


// ======================================================================
//     CONVERSION DU RESULTAT ADC EN TENSION RÉELLE (pile AA/AAA)
// ======================================================================
float lireVBAT() {
  float raw = readADC_stable(PIN_VBAT);                           // Valeur brute de 0 à 1023
  float V = raw * VREF_3V3 / 1023.0;                              // Conversion en tension
  return V / K_DIV;                                               // Ratio toujours =1 ici
}


// ======================================================================
//      MESURE MULTIPLE + DÉTERMINATION DE LA STABILITÉ DE LECTURE
// ======================================================================
bool mesurerStabilite(float *moy, float seuil) {
  const uint8_t N = 6;                                            // Nombre d’échantillons
  float M[N];

  // --- Acquisition multiple ---
  for (uint8_t i = 0; i < N; i++) {
    M[i] = lireVBAT();
    delay(80);                                                    // Laisse respirer la pile (réaction chimique)
  }

  // --- Analyse min/max/moyenne ---
  float sum = 0, minV = M[0], maxV = M[0];

  for (uint8_t i = 0; i < N; i++) {
    sum += M[i];
    if (M[i] < minV) minV = M[i];
    if (M[i] > maxV) maxV = M[i];
  }

  *moy = sum / N;                                                 // Calcul de la moyenne
  return ((maxV - minV) < seuil);                                 // Stabilité si variation < seuil
}


// ======================================================================
//                 CLASSE COULEUR (Rouge / Orange / Vert)
// ======================================================================
int classerPile(float V, float s1, float s2) {
  if (V < s1) return 0;    // Très faible → rouge
  if (V < s2) return 1;    // Moyenne → orange
  return 2;                // Bonne → vert
}


// ======================================================================
//                         BIP BASIQUE DU BUZZER
// ======================================================================
void bipBuzzer(int th, int tl, int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(PIN_BUZZER, HIGH); delay(th);
    digitalWrite(PIN_BUZZER, LOW);  delay(tl);
  }
}



// ======================================================================
//                          SETUP = ONE SHOT
// ======================================================================
void setup() {

  strip.begin();
  strip.clear();
  strip.show();

  pinMode(PIN_CHARGE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  digitalWrite(PIN_CHARGE, LOW);

  // --- Séquence de bienvenue ---
  bipBuzzer(200, 100, 3);
  bipBuzzer(600, 100, 1);
  delay(800);


  // ============================================================
  //      0) DÉTECTION INVERSION POLARITÉ / ABSENCE PILE
  // ============================================================
  float Vinv = lireVBAT();

  if (Vinv < 0.05f) { // ≈ tension zéro → inversion ou aucune pile

    // Clignotement rouge des LED 3 / 7 / 11
    for (uint8_t k = 0; k < 40; k++) {
     
      strip.setPixelColor(3,  strip.Color(200,0,0));

      strip.setPixelColor(7, strip.Color(200,0,0));

      strip.setPixelColor(11, strip.Color(200,0,0));

      strip.show();
      delay(50);

      
      strip.setPixelColor(3, 0);

      strip.setPixelColor(7, 0);
      
      strip.setPixelColor(11, 0);
      strip.show();
      delay(50);
    }

    strip.clear(); strip.show();
    return;   // One-shot : fin du programme
  }



  // ============================================================
  //    1) MESURE À VIDE — Barrette 1
  // ============================================================
  delay(3000); // Laisse la pile se stabiliser avant mesures

  float Vvide;
  bool stableV = mesurerStabilite(&Vvide, 0.015);   // Stabilité
  int nv = classerPile(Vvide, seuil_TresBas_vide, seuil_Acceptable_vide);

  uint32_t colV =
      (nv==0) ? strip.Color(200,0,0) :
      (nv==1) ? strip.Color(255,120,0) :
                strip.Color(0,200,0);

  bipBuzzer(80,50,1);    // 1 bip = test n°1

  strip.setPixelColor(nv, colV); // LED résultat
  strip.setPixelColor(3,          // LED stabilité
                      stableV ? strip.Color(0,40,0)
                              : strip.Color(0,0,200));
  strip.show();

  delay(400);



  // ============================================================
  //    2) MESURE EN CHARGE — Barrette 2
  // ============================================================
  digitalWrite(PIN_CHARGE, HIGH);  // Activation du MOSFET → force courant
  delay(4000);                     // Temps de stabilisation électrique

  float Vcharge;
  bool stableC = mesurerStabilite(&Vcharge, 0.015);

  float Vrs = readADC_stable(PIN_RSENSE) * VREF_3V3 / 1023.0;
  float I = Vrs / RSENSE;  // I = Vrs / Rsense

  delay(700);
  digitalWrite(PIN_CHARGE, LOW); // Fin du stress de charge
  delay(300);

  bipBuzzer(80,50,2); // 2 bips = test n°2

  // Surintensité = pile morte ou court-circuit interne
  if (I > 0.20f) {
    digitalWrite(PIN_CHARGE, LOW);   // <--- OBLIGATOIRE POUR PROTÉGER LA PILE
    strip.fill(strip.Color(200,0,0)); strip.show();
    bipBuzzer(60,80,3); // triple bip alarme
    delay(1500);
    strip.clear(); strip.show();
    return;
  }

  int nc = classerPile(Vcharge, seuil_TresBas_charge, seuil_Acceptable_charge);

  uint32_t colC =
      (nc==0) ? strip.Color(200,0,0) :
      (nc==1) ? strip.Color(255,120,0) :
                strip.Color(0,200,0);

  // LED de résultat barrette 2
  strip.setPixelColor(4 + nc, colC);

  // LED de stabilité barrette 2
  strip.setPixelColor(7,
      stableC ? strip.Color(0,40,0)
              : strip.Color(0,0,200));

  strip.show();



  // ============================================================
  // 3) BARRETTE 3 — ΔV + COHÉRENCE + SURTENSION
  // ============================================================
  delay(3000); // Pausette avant diagnostic final
  bipBuzzer(80,50,3); // 3 bips = test final

  float deltaV = Vvide - Vcharge;  // Écart entre vide et charge

  // Effacement préalable de la barrette 3
  for (int i = 8; i <= 11; i++) strip.setPixelColor(i, 0);

  // ---------- SURTENSION ----------
  if (Vvide > 1.65f) {

    // --- Version C : alternance bleu/rouge 12 fois ---
    for (int i = 0; i < 12; i++) {
      strip.setPixelColor(11, strip.Color(0,0,200)); // bleu
      strip.show();
      delay(100);

      strip.setPixelColor(11, strip.Color(200,0,0)); // rouge
      strip.show();
      delay(100);
    }

    // Rouge fixe final = alarme verrouillée
    strip.setPixelColor(11, strip.Color(200,0,0));
    strip.show();
  }

  // ----------- ANALYSE NORMALE ----------
  else {

    // Cas excellent : cohérence parfaite + faible ΔV
    if (nv==2 && nc==2 && deltaV < 0.25f)
      strip.setPixelColor(10, strip.Color(0,200,0));

    // Cas moyen : cohérent mais ΔV modéré
    else if (nv>0 && nc>0 && deltaV < 0.30f)
      strip.setPixelColor(9, strip.Color(255,120,0));

    // Cas mauvais : incohérence ou ΔV trop grand
    else
      strip.setPixelColor(8, strip.Color(200,0,0));

    strip.show();
  }

  
}



// ======================================================================
//       LOOP VIDE = vrai ONE SHOT (un seul test par RESET hardware)
// ======================================================================
void loop() {
  // rien → tout s'exécute une seule fois dans setup()
}
