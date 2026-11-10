# Consignes d'architecture et de qualité — Projet SplitIUT

## Stack technique
- Langage : C++17, CMake ≥ 3.14 (compilable avec MSVC, GCC et Clang)
- Tests unitaires : GoogleTest 1.15.2 (FetchContent, déjà présent dans `CMakeLists.txt`)
- JSON : nlohmann/json (en-tête unique, FetchContent), seule dépendance autorisée en plus de GoogleTest

## Architecture stricte en couches (OBLIGATOIRE)
1. `src/domain/` : logique métier pure (modèles, répartition des dépenses, calcul des soldes,
   résolution des remboursements, conversion texte `10,00` ↔ `Montant`). ZÉRO `std::cout`/`std::cin`,
   ZÉRO `<iostream>`/`<fstream>`, ZÉRO JSON. N'inclut rien d'autre de `src/`.
2. `src/storage/` : sauvegarde et chargement du groupe en JSON local. Dépend uniquement de `domain`.
   Le chargement reconstruit le groupe en appelant les fonctions du domaine : un fichier invalide est
   rejeté par les mêmes validations que la saisie.
3. `src/cli/` : interface console (`main.cpp`), seul endroit autorisé à lire et écrire sur la console.
   Appelle `domain` et `storage`, ne contient aucune règle métier.
4. `tests/` : tests GoogleTest ciblant prioritairement `src/domain/`, incluant les deux exemples
   de la section 5 de `SPEC.md`.

Dépendances imposées par CMake (une couche ne peut pas inclure ce qu'elle ne lie pas) :

| Cible | Type | Liée à |
|---|---|---|
| `splitiut_domain` | bibliothèque statique | rien |
| `splitiut_storage` | bibliothèque statique | `splitiut_domain`, `nlohmann_json` |
| `splitiut` | exécutable | `splitiut_storage`, `splitiut_domain` |
| `tests` | exécutable de test (existant) | `splitiut_domain`, `splitiut_storage`, `GTest::gtest_main` |

## Format JSON

```json
{
  "nom": "Week-end BDE",
  "participants": ["Alice", "Bob", "Chloé"],
  "depenses": [
    { "titre": "Pizzas", "montant": 1000, "payeur": "Bob", "mode": "EGAL",
      "beneficiaires": [{ "nom": "Alice", "poids": 1 }, { "nom": "Bob", "poids": 1 },
                        { "nom": "Chloé", "poids": 1 }] }
  ]
}
```

- `montant` : entier de centimes, jamais `10.00` ni `"10,00"`. Un nombre non entier est rejeté au chargement.
- L'ordre du tableau `participants` est l'ordre d'ajout : il doit être conservé, car il décide
  de l'attribution des centimes restants.
- Les parts et les soldes ne sont pas stockés : ils sont recalculés.

## Règles de code impératives
- INTERDICTION des flottants (`float`/`double`) pour les montants : tous les calculs se font en
  centimes entiers `std::int64_t` (pas `int` : sur 32 bits, il déborde au-delà de 21 474 836,47 €).
- Respecter strictement le contrat défini dans `SPEC.md`.
- Erreurs : le domaine signale tout rejet par `std::invalid_argument` avec un message explicite.
  Seule la CLI attrape et affiche ces erreurs. Aucun `exit()` hors de `main.cpp`.
- Sources en UTF-8 ; option `/utf-8` pour MSVC (sinon `"Chloé"` dans le code ≠ `"Chloé"` lu du JSON).
- Code simple et typé : pas d'héritage ni de patron de conception non requis par `SPEC.md`.
- Ne pas modifier `benchmark/` ni le rapport d'atelier.
