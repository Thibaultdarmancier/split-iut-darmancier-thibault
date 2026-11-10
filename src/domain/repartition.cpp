#include "repartition.hpp"

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <stdexcept>

namespace splitiut {

std::vector<Montant> repartir(Montant montant, const std::vector<PoidsBeneficiaire>& beneficiaires) {
    if (beneficiaires.empty()) {
        throw std::invalid_argument("Répartition impossible : aucun bénéficiaire");
    }
    // M <= 10^8 et p_i <= INT_MAX : M × p_i ne déborde pas (SPEC §2).
    verifierBornesMontant(montant);

    std::int64_t totalPoids = 0;
    for (const PoidsBeneficiaire& beneficiaire : beneficiaires) {
        if (beneficiaire.poids <= 0) {
            throw std::invalid_argument("Répartition impossible : poids <= 0");
        }
        totalPoids += beneficiaire.poids;
    }

    // 1. Parts tronquées et restes.
    const std::int64_t m = montant.centimes();
    const std::size_t n = beneficiaires.size();
    std::vector<std::int64_t> parts(n);
    std::vector<std::int64_t> restes(n);
    std::int64_t distribue = 0;
    for (std::size_t i = 0; i < n; ++i) {
        const std::int64_t produit = m * beneficiaires[i].poids;
        parts[i] = produit / totalPoids;
        restes[i] = produit % totalPoids;
        distribue += parts[i];
    }

    // 2. Centimes restants : r < n (SPEC §2).
    const auto r = static_cast<std::size_t>(m - distribue);

    // 3. +1 centime aux r premiers : reste décroissant, puis ordre d'ajout au groupe.
    std::vector<std::size_t> ordre(n);
    std::iota(ordre.begin(), ordre.end(), std::size_t{0});
    std::sort(ordre.begin(), ordre.end(), [&](std::size_t a, std::size_t b) {
        if (restes[a] != restes[b]) {
            return restes[a] > restes[b];
        }
        return beneficiaires[a].rang < beneficiaires[b].rang;
    });
    for (std::size_t k = 0; k < r; ++k) {
        parts[ordre[k]] += 1;
    }

    std::vector<Montant> resultat;
    resultat.reserve(n);
    for (std::int64_t part : parts) {
        resultat.emplace_back(part);
    }
    return resultat;
}

std::vector<Montant> calculerParts(const Groupe& groupe, const Depense& depense) {
    std::vector<PoidsBeneficiaire> poids;
    poids.reserve(depense.beneficiaires.size());
    for (const Beneficiaire& beneficiaire : depense.beneficiaires) {
        poids.push_back(PoidsBeneficiaire{groupe.rangDe(beneficiaire.nom), beneficiaire.poids});
    }
    return repartir(depense.montant, poids);
}

}  // namespace splitiut
