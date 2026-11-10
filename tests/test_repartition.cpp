#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "exemples_spec.hpp"
#include "groupe.hpp"
#include "repartition.hpp"

using exemples::depense;
using splitiut::Groupe;
using splitiut::ModeRepartition;
using splitiut::Montant;
using splitiut::PoidsBeneficiaire;
using splitiut::repartir;

namespace {

std::vector<std::int64_t> enCentimes(const std::vector<Montant>& parts) {
    std::vector<std::int64_t> resultat;
    for (Montant part : parts) {
        resultat.push_back(part.centimes());
    }
    return resultat;
}

std::vector<std::int64_t> repartirCentimes(std::int64_t montant,
                                           const std::vector<PoidsBeneficiaire>& beneficiaires) {
    return enCentimes(repartir(Montant(montant), beneficiaires));
}

using Centimes = std::vector<std::int64_t>;

}  // namespace

TEST(Repartition, EgalDixEurosEntreTrois) {
    EXPECT_EQ(repartirCentimes(1000, {{0, 1}, {1, 1}, {2, 1}}), (Centimes{334, 333, 333}));
}

TEST(Repartition, PondereDixCentimesPoids1Et2) {
    EXPECT_EQ(repartirCentimes(10, {{0, 1}, {1, 2}}), (Centimes{3, 7}));
}

TEST(Repartition, UnCentimeEntreTrois) {
    EXPECT_EQ(repartirCentimes(1, {{0, 1}, {1, 1}, {2, 1}}), (Centimes{1, 0, 0}));
}

TEST(Repartition, PlusGrandResteServiEnPremier) {
    // SPEC §5 exemple 2, D1 : restes 2 / 4 / 0 -> le centime va à Bob.
    EXPECT_EQ(repartirCentimes(10000, {{0, 2}, {1, 1}, {3, 3}}), (Centimes{3333, 1667, 5000}));
}

TEST(Repartition, EgaliteDeRestesDepartageeParOrdreDAjout) {
    // Bénéficiaires saisis à l'envers : le centime va quand même au rang 0.
    EXPECT_EQ(repartirCentimes(1000, {{2, 1}, {1, 1}, {0, 1}}), (Centimes{333, 333, 334}));
    EXPECT_EQ(repartirCentimes(2, {{2, 1}, {0, 1}, {1, 1}}), (Centimes{0, 1, 1}));
}

TEST(Repartition, SommeDesPartsEgaleAuMontant) {
    const std::vector<std::vector<PoidsBeneficiaire>> cas = {
        {{0, 1}},
        {{0, 1}, {1, 1}, {2, 1}},
        {{0, 2}, {1, 1}, {2, 3}},
        {{0, 7}, {1, 3}, {2, 5}, {3, 11}},
        {{0, 1}, {1, 1000000}},
    };
    for (const auto& beneficiaires : cas) {
        for (std::int64_t montant : {1, 2, 7, 99, 100, 1001, 33333, 99999999, 100000000}) {
            std::int64_t somme = 0;
            for (std::int64_t part : repartirCentimes(montant, beneficiaires)) {
                EXPECT_GE(part, 0);
                somme += part;
            }
            EXPECT_EQ(somme, montant);
        }
    }
}

TEST(Repartition, PreconditionsRejetees) {
    EXPECT_THROW(repartir(Montant(1000), {}), std::invalid_argument);
    EXPECT_THROW(repartir(Montant(1000), {{0, 1}, {1, 0}}), std::invalid_argument);
    EXPECT_THROW(repartir(Montant(0), {{0, 1}}), std::invalid_argument);
}

TEST(CalculerParts, MemeResultatQuelQueSoitLOrdreDeSaisie) {
    Groupe groupe("Week-end BDE");
    groupe.ajouterParticipant("Alice");
    groupe.ajouterParticipant("Bob");
    groupe.ajouterParticipant("Chloé");

    const auto ordreAjout = splitiut::calculerParts(
        groupe, depense("Pizzas", "Bob", 1000, ModeRepartition::EGAL,
                        {{"Alice", 1}, {"Bob", 1}, {"Chloé", 1}}));
    const auto ordreInverse = splitiut::calculerParts(
        groupe, depense("Pizzas", "Bob", 1000, ModeRepartition::EGAL,
                        {{"Chloé", 1}, {"Bob", 1}, {"Alice", 1}}));

    // Alice reçoit le centime dans les deux cas.
    EXPECT_EQ(enCentimes(ordreAjout), (Centimes{334, 333, 333}));
    EXPECT_EQ(enCentimes(ordreInverse), (Centimes{333, 333, 334}));
}

TEST(CalculerParts, Exemple2) {
    const Groupe groupe = exemples::exemple2();
    EXPECT_EQ(enCentimes(splitiut::calculerParts(groupe, groupe.depenses()[0])),
              (Centimes{3333, 1667, 5000}));
    EXPECT_EQ(enCentimes(splitiut::calculerParts(groupe, groupe.depenses()[1])),
              (Centimes{2334, 2333, 2333}));
}
