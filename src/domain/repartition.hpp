#pragma once

#include <cstddef>
#include <vector>

#include "depense.hpp"
#include "groupe.hpp"
#include "montant.hpp"

namespace splitiut {

struct PoidsBeneficiaire {
    std::size_t rang;  // ordre d'ajout au groupe : départage les restes égaux
    int poids;
};

// Algorithme unique de répartition au centime (SPEC §2) :
// part_i = ⌊M × p_i / P⌋, puis les centimes restants vont aux plus grands restes,
// à égalité dans l'ordre d'ajout au groupe. Σ parts = montant exactement.
// Retourne les parts dans l'ordre de `beneficiaires`.
// Lève std::invalid_argument si la liste est vide, si un poids est <= 0
// ou si le montant est hors bornes.
std::vector<Montant> repartir(Montant montant, const std::vector<PoidsBeneficiaire>& beneficiaires);

// Parts d'une dépense du groupe, dans l'ordre de depense.beneficiaires.
std::vector<Montant> calculerParts(const Groupe& groupe, const Depense& depense);

}  // namespace splitiut
