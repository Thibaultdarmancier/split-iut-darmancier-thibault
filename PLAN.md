# PLAN — SplitIUT v1

Règles communes à chaque étape :
- Une étape = une demande à l'IA, avec `SPEC.md` et `AGENTS.md` fournis en contexte.
- Étape terminée quand : `cmake --build build` réussit, `ctest --test-dir build` est entièrement vert,
  chaque ligne générée a été relue, puis commit.
- On ne commence l'étape suivante que sur une base verte. Les nouveaux fichiers de test sont ajoutés à la cible `tests`.

## Étape 1 — Modèles et participants (SF1)
- **Périmètre :** type `Montant` (centimes `std::int64_t`, conversion texte `10,00` ↔ `Montant`),
  `Participant`, `Groupe` (création, ajout d'un participant).
- **Règles SPEC :** §1 ; §3 (noms, doublons, format et bornes des montants).
- **Fichiers :** `src/domain/montant.hpp`, `src/domain/montant.cpp`, `src/domain/groupe.hpp`,
  `src/domain/groupe.cpp`, `tests/test_montant.cpp`, `tests/test_groupe.cpp`,
  `CMakeLists.txt` (cible `splitiut_domain`, option `/utf-8` pour MSVC).
- **Vérification :** `10,00` → 1000 ; `10`, `10,5`, `10.50`, `10,005`, `0,00`, `1000000,01` rejetés ;
  `0,01` et `1000000,00` acceptés ; nom vide, `"   "`, `alice` après `Alice` rejetés ; ordre d'ajout conservé.

## Étape 2 — Dépenses et répartition au centime (SF2, SF3)
- **Périmètre :** `Depense` (modes `EGAL` / `PONDERE`), validation et ajout atomique au groupe,
  algorithme unique de répartition (plus grand reste, puis ordre d'ajout).
- **Règles SPEC :** §2 ; §3 (titre, payeur, bénéficiaires, poids).
- **Fichiers :** `src/domain/depense.hpp`, `src/domain/depense.cpp`, `src/domain/repartition.hpp`,
  `src/domain/repartition.cpp`, `src/domain/groupe.hpp` et `src/domain/groupe.cpp` (ajout d'une dépense),
  `tests/test_repartition.cpp`, `tests/test_depense.cpp`.
- **Vérification :** 10,00 € / 3 → 3,34 / 3,33 / 3,33 ; 0,10 € avec poids 1 et 2 → 0,03 / 0,07 ;
  égalité de restes → ordre d'ajout ; même résultat quel que soit l'ordre de saisie ; Σ parts = montant ;
  0,01 € entre 3 → 0,01 / 0,00 / 0,00 ; dépense invalide rejetée et groupe inchangé.

## Étape 3 — Calcul des soldes (SF4)
- **Périmètre :** total payé, total consommé et solde net de chaque participant ; invariant Σ soldes = 0.
- **Règles SPEC :** §4 (soldes).
- **Fichiers :** `src/domain/bilan.hpp`, `src/domain/bilan.cpp`, `tests/test_bilan.cpp`.
- **Vérification :** soldes des exemples 1 et 2 de `SPEC.md` §5 ; Σ soldes = 0 après chaque dépense ;
  payeur non bénéficiaire ; groupe sans dépense → tous les soldes à 0,00.

## Étape 4 — Virements simplifiés (SF5)
- **Périmètre :** algorithme glouton et tri d'affichage des virements.
- **Règles SPEC :** §4 (virements).
- **Fichiers :** `src/domain/remboursement.hpp`, `src/domain/remboursement.cpp`, `tests/test_remboursement.cpp`.
- **Vérification :** virements des exemples 1 et 2 ; soldes Alice +10 / Bob +5 / Chloé −8 / Dan −7 →
  3 virements ; soldes tous nuls → plan vide ; au plus n − 1 virements ; appliquer le plan remet tous les soldes à 0.

## Étape 5 — Sauvegarde et chargement JSON
- **Périmètre :** écriture et lecture du groupe au format défini dans `AGENTS.md` ; le chargement
  reconstruit le groupe via les fonctions du domaine.
- **Fichiers :** `src/storage/stockage_json.hpp`, `src/storage/stockage_json.cpp`, `tests/test_stockage.cpp`,
  `CMakeLists.txt` (nlohmann/json, cible `splitiut_storage`).
- **Vérification :** sauvegarde puis chargement → mêmes soldes et virements ; ordre des participants conservé ;
  montant `10.5` ou `"10,00"` rejeté ; doublon ou payeur inconnu dans le fichier rejeté ; fichier absent → erreur explicite.

## Étape 6 — Interface CLI
- **Périmètre :** menu console : créer le groupe, ajouter un participant, ajouter une dépense,
  afficher le bilan, afficher les virements, sauvegarder, charger. Aucune règle métier.
- **Fichiers :** `src/cli/main.cpp`, `CMakeLists.txt` (exécutable `splitiut`).
- **Vérification :** scénario manuel de l'exemple 1 → affichage identique à `SPEC.md` §5 ; saisie invalide →
  message d'erreur et programme toujours actif ; sauvegarde puis rechargement dans une nouvelle exécution → même bilan.
