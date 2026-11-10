#include <gtest/gtest.h>

#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "bilan.hpp"
#include "exemples_spec.hpp"
#include "remboursement.hpp"

using splitiut::calculerBilan;
using splitiut::calculerVirements;
using splitiut::LigneBilan;
using splitiut::Montant;
using splitiut::Virement;

namespace {

LigneBilan ligne(const std::string& nom, std::int64_t solde) {
    return LigneBilan{nom, Montant(), Montant(), Montant(solde)};
}

void verifierVirement(const Virement& virement, const std::string& debiteur,
                      const std::string& crediteur, std::int64_t centimes) {
    EXPECT_EQ(virement.debiteur, debiteur);
    EXPECT_EQ(virement.crediteur, crediteur);
    EXPECT_EQ(virement.montant.centimes(), centimes);
}

// Au plus n − 1 virements, et appliquer le plan remet tous les soldes à 0.
void verifierPlan(const std::vector<LigneBilan>& bilan) {
    const std::vector<Virement> virements = calculerVirements(bilan);
    if (!bilan.empty()) {
        EXPECT_LE(virements.size(), bilan.size() - 1);
    }

    std::map<std::string, std::int64_t> soldes;
    for (const LigneBilan& l : bilan) {
        soldes[l.nom] = l.solde.centimes();
    }
    for (const Virement& v : virements) {
        EXPECT_GT(v.montant.centimes(), 0);
        soldes[v.debiteur] += v.montant.centimes();
        soldes[v.crediteur] -= v.montant.centimes();
    }
    for (const auto& solde : soldes) {
        EXPECT_EQ(solde.second, 0) << solde.first;
    }
}

}  // namespace

TEST(Virements, Exemple1) {
    const auto virements = calculerVirements(calculerBilan(exemples::exemple1()));
    ASSERT_EQ(virements.size(), 2u);
    verifierVirement(virements[0], "Bob", "Alice", 333);
    verifierVirement(virements[1], "Chloé", "Alice", 1333);
}

TEST(Virements, Exemple2) {
    const auto virements = calculerVirements(calculerBilan(exemples::exemple2()));
    ASSERT_EQ(virements.size(), 3u);
    verifierVirement(virements[0], "Alice", "Chloé", 5667);
    verifierVirement(virements[1], "Bob", "Chloé", 2000);  // Chloé et Dan ex aequo : Chloé d'abord
    verifierVirement(virements[2], "Bob", "Dan", 2000);
}

TEST(Virements, QuatreParticipantsTroisVirements) {
    const auto virements = calculerVirements(
        {ligne("Alice", 1000), ligne("Bob", 500), ligne("Chloé", -800), ligne("Dan", -700)});
    ASSERT_EQ(virements.size(), 3u);
    verifierVirement(virements[0], "Chloé", "Alice", 800);
    verifierVirement(virements[1], "Dan", "Alice", 200);
    verifierVirement(virements[2], "Dan", "Bob", 500);
}

TEST(Virements, SoldesTousNulsPlanVide) {
    EXPECT_TRUE(calculerVirements({ligne("Alice", 0), ligne("Bob", 0)}).empty());
    EXPECT_TRUE(calculerVirements({}).empty());
}

TEST(Virements, AuPlusNMoins1EtPlanSoldeTout) {
    verifierPlan(calculerBilan(exemples::exemple1()));
    verifierPlan(calculerBilan(exemples::exemple2()));
    verifierPlan({ligne("Alice", 1000), ligne("Bob", 500), ligne("Chloé", -800), ligne("Dan", -700)});
    verifierPlan({ligne("A", -1), ligne("B", -1), ligne("C", -1), ligne("D", 3)});
    verifierPlan({ligne("A", 5), ligne("B", -2), ligne("C", 0), ligne("D", -3), ligne("E", 7),
                  ligne("F", -7)});
    verifierPlan({ligne("A", 100000000), ligne("B", -33333333), ligne("C", -33333333),
                  ligne("D", -33333334)});
}

TEST(Virements, SommeDesSoldesNonNulleRejetee) {
    EXPECT_THROW(calculerVirements({ligne("Alice", 100), ligne("Bob", -99)}), std::invalid_argument);
}
