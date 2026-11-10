#include "remboursement.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace splitiut {

namespace {

constexpr std::size_t AUCUN = static_cast<std::size_t>(-1);

struct VirementIndexe {
    std::size_t debiteur;
    std::size_t crediteur;
    std::int64_t montant;
};

}  // namespace

std::vector<Virement> calculerVirements(const std::vector<LigneBilan>& bilan) {
    std::vector<std::int64_t> soldes;
    soldes.reserve(bilan.size());
    std::int64_t somme = 0;
    for (const LigneBilan& ligne : bilan) {
        soldes.push_back(ligne.solde.centimes());
        somme += ligne.solde.centimes();
    }
    if (somme != 0) {
        throw std::invalid_argument("Bilan incohérent : la somme des soldes doit être nulle");
    }

    std::vector<VirementIndexe> plan;
    while (true) {
        // Soldes nuls ignorés ; comparaison stricte : à égalité, le premier ajouté l'emporte.
        std::size_t debiteur = AUCUN;
        std::size_t crediteur = AUCUN;
        for (std::size_t i = 0; i < soldes.size(); ++i) {
            if (soldes[i] < 0 && (debiteur == AUCUN || soldes[i] < soldes[debiteur])) {
                debiteur = i;
            }
            if (soldes[i] > 0 && (crediteur == AUCUN || soldes[i] > soldes[crediteur])) {
                crediteur = i;
            }
        }
        if (debiteur == AUCUN) {
            break;  // Σ soldes = 0 : plus de débiteur implique plus de créditeur.
        }

        const std::int64_t montant = std::min(-soldes[debiteur], soldes[crediteur]);
        soldes[debiteur] += montant;
        soldes[crediteur] -= montant;
        plan.push_back(VirementIndexe{debiteur, crediteur, montant});
    }

    std::sort(plan.begin(), plan.end(), [](const VirementIndexe& a, const VirementIndexe& b) {
        if (a.debiteur != b.debiteur) {
            return a.debiteur < b.debiteur;
        }
        return a.crediteur < b.crediteur;
    });

    std::vector<Virement> virements;
    virements.reserve(plan.size());
    for (const VirementIndexe& virement : plan) {
        virements.push_back(Virement{bilan[virement.debiteur].nom, bilan[virement.crediteur].nom,
                                     Montant(virement.montant)});
    }
    return virements;
}

}  // namespace splitiut
