# TESTEUR-DE-PILES-REALISATION-PCB-DFTA248

🔋 Testeur de piles AA / AAA intelligent — ATtiny85

Projet de testeur de piles AA et AAA 1,5 V basé sur un microcontrôleur ATtiny85, intégrant une mesure à vide, une mesure sous charge contrôlée et une analyse de cohérence afin de fournir un diagnostic réaliste de l’état de la pile.

👉 Ce projet est open hardware / open source : schéma, code et documentation sont fournis.

🎯 Objectif du projet

Les testeurs de piles courants se limitent à une mesure de tension à vide, souvent trompeuse.
Une pile peut afficher une tension correcte sans être capable de fournir du courant.

Ce testeur a été conçu pour :

simuler une utilisation réelle de la pile,

observer son comportement sous charge,

analyser la stabilité de la tension,

comparer les résultats pour fournir un diagnostic fiable.

⚙️ Fonctionnalités principales

✔️ Mesure de la tension à vide

✔️ Mesure de la tension sous charge (~100 mA)

✔️ Analyse de stabilité par lectures multiples

✔️ Diagnostic final basé sur ΔV (vide / charge)

✔️ Affichage clair par LEDs adressables

✔️ Signalisation sonore (buzzer)

✔️ Protection contre :

inversion de polarité

surtension

surintensité

✔️ Fonctionnement one-shot (un reset matériel = un test)

🧠 Principe de fonctionnement

Le test se déroule automatiquement en trois phases, signalées par des bips sonores :

🔹 Phase 1 — Tension à vide

Évalue la tension “brute” de la pile

Donne une première indication de son état apparent

🔹 Phase 2 — Tension en charge

Applique une charge électronique contrôlée

Révèle la résistance interne et la capacité réelle de la pile

🔹 Phase 3 — Diagnostic final

Compare les deux mesures

Analyse l’écart ΔV

Détermine la cohérence du comportement

💡 Affichage par LEDs adressables

Le testeur utilise 3 barrettes de LEDs WS2812B (développement : 3×8, version finale : 3×4).

Barrette 1 — Tension à vide

🔴 Rouge : pile très faible

🟠 Orange : pile moyenne

🟢 Vert : pile bonne

LED stabilité :

🟢 Vert : tension stable

🔵 Bleu : tension instable

Barrette 2 — Tension en charge

🔴 Rouge : pile inutilisable sous charge

🟠 Orange : pile fatiguée

🟢 Vert : pile en bon état

LED stabilité :

🟢 Vert : comportement stable

🔵 Bleu : chute ou instabilité

Barrette 3 — Diagnostic final

🟢 Vert : pile excellente

🟠 Orange : pile utilisable mais affaiblie

🔴 Rouge : pile à remplacer

Cas particuliers :

🔵 / 🔴 clignotant : inversion de polarité ou pile absente

🔵 puis 🔴 fixe : surtension détectée

🔊 Signalisation sonore

1 bip → test à vide

2 bips → test en charge

3 bips → diagnostic final

Bips rapides → alarme / sécurité

🛡️ Sécurités intégrées

Inversion de polarité détectée et signalée

Coupure immédiate en cas de surintensité

Surveillance de surtension

Protection des entrées analogiques

Charge désactivée en fin de test

🔌 Alimentation

Alimentation externe 5 V :

chargeur USB

power bank

alimentation de laboratoire

Régulation locale 3,3 V via LDO MCP1700 pour :

stabilité de l’ADC

précision des mesures

🧪 Microcontrôleur

ATtiny85

Horloge interne 8 MHz

Reset matériel via PB5

Code Arduino (ATTinyCore)

📂 Contenu du dépôt

/schematics → schéma électronique

/pcb → PCB (à venir / version finale dans une prochaine vidéo)

/firmware → code ATtiny85

/docs → guide utilisateur et documentation

⚠️ Avertissements

❌ Ce testeur n’est pas un chargeur

❌ Conçu uniquement pour piles AA / AAA 1,5 V

ℹ️ Les LEDs donnent un diagnostic fonctionnel, pas une capacité en mAh

📺 Vidéos associées

Présentation du schéma et démonstration sur breadboard

Vidéo suivante : PCB final et réalisation complète

👉 Chaîne YouTube :
https://www.youtube.com/c/ClaudeDufourmont

📜 Licence

Projet libre à usage personnel et pédagogique.
Merci de citer l’auteur en cas de réutilisation ou adaptation.
