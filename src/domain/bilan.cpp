#include "bilan.hpp"

#include "repartition.hpp"

namespace splitiut {

std::vector<LigneBilan> calculerBilan(const Groupe& groupe) {
    std::vector<LigneBilan> bilan;
    bilan.reserve(groupe.participants().size());
    for (const Participant& participant : groupe.participants()) {
        bilan.push_back(LigneBilan{participant.nom, Montant(), Montant(), Montant()});
    }

    for (const Depense& depense : groupe.depenses()) {
        bilan[groupe.rangDe(depense.payeur)].paye += depense.montant;

        const std::vector<Montant> parts = calculerParts(groupe, depense);
        for (std::size_t i = 0; i < parts.size(); ++i) {
            bilan[groupe.rangDe(depense.beneficiaires[i].nom)].consomme += parts[i];
        }
    }

    for (LigneBilan& ligne : bilan) {
        ligne.solde = ligne.paye - ligne.consomme;
    }
    return bilan;
}

}  // namespace splitiut
