# SPEC — SplitIUT v1

C++ (CMake + GoogleTest). En cas de conflit, le cahier des charges prime.

## 1. Modèle de données

Tous les montants sont des **centimes entiers** (`int64_t`) encapsulés dans un type `Montant`. Aucun `double`.

| Entité | Champs |
|---|---|
| `Groupe` | `nom` (texte) ; `participants` (liste, ordre d'ajout) ; `depenses` (liste) |
| `Participant` | `nom` (espaces de début/fin retirés, identifiant unique dans le groupe) ; son rang dans `participants` = ordre d'ajout |
| `Depense` | `titre` ; `montant` (`Montant`) ; `payeur` (Participant) ; `mode` (`EGAL` ou `PONDERE`) ; `beneficiaires` (liste de couples Participant / poids entier, poids = 1 en `EGAL`) |

Périmètre v1 : un seul groupe ; pas de renommage ni suppression de participant ; pas de modification ni suppression de dépense.

## 2. Répartition et arrondi

Un seul algorithme : le partage égal est un partage pondéré où tous les poids valent 1.
Soit M le montant en centimes, p_i le poids du bénéficiaire i, P = Σ p_i :

1. part_i = ⌊M × p_i / P⌋ et reste_i = (M × p_i) mod P
2. r = M − Σ part_i
3. Les r premiers bénéficiaires reçoivent +1 centime, triés par : ① reste_i décroissant, ② ordre d'ajout au groupe (jamais l'ordre de saisie).

Garantie : Σ part_i × P + Σ reste_i = M × P, donc r = Σ reste_i / P est entier et inférieur au nombre de bénéficiaires. Après l'étape 3, Σ parts = M exactement.

- Égal : 10,00 € entre 3 → restes égaux → 3,34 / 3,33 / 3,33 dans l'ordre d'ajout.
- Pondéré : 0,10 € avec Alice = 1, Bob = 2 → restes 1 et 2 → Alice 0,03, Bob 0,07.
- Pas de débordement : M ≤ 10⁸ et p_i ≤ INT_MAX, donc M × p_i < 9,2·10¹⁸.
- Une part de 0,00 € est autorisée (0,01 € entre 3 → 0,01 / 0,00 / 0,00).

## 3. Validation et erreurs rejetées

Une saisie invalide est rejetée entièrement ; l'état du groupe reste inchangé.

| Élément | Rejeté si |
|---|---|
| Nom (groupe ou participant) | vide après retrait des espaces |
| Participant | doublon, insensible à la casse ASCII (`alice` = `Alice`), accents distincts (`Alíce` ≠ `Alice`) |
| Montant | format autre que `chiffres,2 chiffres` (`10`, `10,5`, `10.50`, `10,005`, `10,00 €`) ; ≤ 0 ; > 1 000 000,00 € |
| Titre | vide après retrait des espaces |
| Payeur | absent du groupe (il peut ne pas être bénéficiaire) |
| Bénéficiaires | liste vide ; participant absent du groupe ; doublon dans la liste |
| Poids (`PONDERE`) | ≤ 0 |

## 4. Soldes et virements

- payé_i = Σ montants des dépenses payées par i
- consommé_i = Σ parts de i (section 2)
- solde_i = payé_i − consommé_i (> 0 : à rembourser ; < 0 : doit de l'argent) ; invariant : Σ solde_i = 0

Virements (glouton) :
1. Ignorer les soldes nuls.
2. Tant qu'il reste un débiteur : le plus gros débiteur verse au plus gros créditeur min(|dette|, créance). Égalités départagées par ordre d'ajout.
3. Afficher les virements triés par ordre d'ajout du débiteur, puis du créditeur.
4. Garantie : au plus n − 1 virements (pas forcément le minimum absolu, problème NP-difficile hors périmètre).

## 5. Exemples chiffrés

### Exemple 1 — cahier des charges

Groupe : Alice, Bob, Chloé.
- D1 : Alice paie 30,00 €, égal entre Alice, Bob, Chloé → 10,00 / 10,00 / 10,00.
- D2 : Bob paie 10,00 €, égal entre Alice, Bob, Chloé → 3,34 / 3,33 / 3,33.

| | Payé | Consommé | Solde |
|---|---|---|---|
| Alice | 30,00 | 13,34 | +16,66 |
| Bob | 10,00 | 13,33 | −3,33 |
| Chloé | 0,00 | 13,33 | −13,33 |

Virements : Bob → Alice : 3,33 ; Chloé → Alice : 13,33.

### Exemple 2 — pondéré, payeurs non bénéficiaires, égalité

Groupe : Alice, Bob, Chloé, Dan.
- D1 : Chloé paie 100,00 €, pondéré Alice = 2, Bob = 1, Dan = 3 (P = 6).
  Parts tronquées 33,33 / 16,66 / 50,00, restes 2 / 4 / 0, r = 1 → Bob. Parts : 33,33 / 16,67 / 50,00.
- D2 : Dan paie 70,00 €, égal entre Alice, Bob, Chloé → 23,34 / 23,33 / 23,33.

| | Payé | Consommé | Solde |
|---|---|---|---|
| Alice | 0,00 | 56,67 | −56,67 |
| Bob | 0,00 | 40,00 | −40,00 |
| Chloé | 100,00 | 23,33 | +76,67 |
| Dan | 70,00 | 50,00 | +20,00 |

Virements : Alice → Chloé : 56,67 ; Bob → Chloé : 20,00 ; Bob → Dan : 20,00 (après le 1er virement, Chloé et Dan sont ex aequo à +20,00 → Chloé, ajoutée avant).
