#pragma once

#include <string>
#include <vector>

#include "bilan.hpp"
#include "montant.hpp"

namespace splitiut {

struct Virement {
    std::string debiteur;
    std::string crediteur;
    Montant montant;
};

// Plan glouton (SPEC §4) à partir d'un bilan dans l'ordre d'ajout (celui de calculerBilan) :
// le plus gros débiteur verse au plus gros créditeur min(|dette|, créance), égalités
// départagées par ordre d'ajout. Au plus n − 1 virements, triés par ordre d'ajout
// du débiteur puis du créditeur.
// Lève std::invalid_argument si la somme des soldes n'est pas nulle.
std::vector<Virement> calculerVirements(const std::vector<LigneBilan>& bilan);

}  // namespace splitiut
