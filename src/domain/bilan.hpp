#pragma once

#include <string>
#include <vector>

#include "groupe.hpp"
#include "montant.hpp"

namespace splitiut {

struct LigneBilan {
    std::string nom;
    Montant paye;      // Σ montants des dépenses payées
    Montant consomme;  // Σ parts reçues (SPEC §2)
    Montant solde;     // paye - consomme : > 0 à rembourser, < 0 doit de l'argent
};

// Une ligne par participant, dans l'ordre d'ajout (SPEC §4). Invariant : Σ soldes = 0.
std::vector<LigneBilan> calculerBilan(const Groupe& groupe);

}  // namespace splitiut
