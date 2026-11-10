#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "bilan.hpp"
#include "exemples_spec.hpp"
#include "groupe.hpp"

using exemples::depense;
using splitiut::calculerBilan;
using splitiut::Groupe;
using splitiut::LigneBilan;
using splitiut::ModeRepartition;

namespace {

void verifierLigne(const LigneBilan& ligne, const std::string& nom, std::int64_t paye,
                   std::int64_t consomme, std::int64_t solde) {
    EXPECT_EQ(ligne.nom, nom);
    EXPECT_EQ(ligne.paye.centimes(), paye) << nom;
    EXPECT_EQ(ligne.consomme.centimes(), consomme) << nom;
    EXPECT_EQ(ligne.solde.centimes(), solde) << nom;
}

std::int64_t sommeDesSoldes(const std::vector<LigneBilan>& bilan) {
    std::int64_t somme = 0;
    for (const LigneBilan& ligne : bilan) {
        somme += ligne.solde.centimes();
    }
    return somme;
}

}  // namespace

TEST(Bilan, Exemple1) {
    const auto bilan = calculerBilan(exemples::exemple1());
    ASSERT_EQ(bilan.size(), 3u);
    verifierLigne(bilan[0], "Alice", 3000, 1334, 1666);
    verifierLigne(bilan[1], "Bob", 1000, 1333, -333);
    verifierLigne(bilan[2], "Chloé", 0, 1333, -1333);
}

TEST(Bilan, Exemple2) {
    const auto bilan = calculerBilan(exemples::exemple2());
    ASSERT_EQ(bilan.size(), 4u);
    verifierLigne(bilan[0], "Alice", 0, 5667, -5667);
    verifierLigne(bilan[1], "Bob", 0, 4000, -4000);
    verifierLigne(bilan[2], "Chloé", 10000, 2333, 7667);
    verifierLigne(bilan[3], "Dan", 7000, 5000, 2000);
}

TEST(Bilan, SommeDesSoldesNulleApresChaqueDepense) {
    Groupe groupe("Week-end BDE");
    for (const char* nom : {"Alice", "Bob", "Chloé", "Dan"}) {
        groupe.ajouterParticipant(nom);
    }
    const std::vector<splitiut::Depense> depenses = {
        depense("D1", "Chloé", 10000, ModeRepartition::PONDERE, {{"Alice", 2}, {"Bob", 1}, {"Dan", 3}}),
        depense("D2", "Dan", 7000, ModeRepartition::EGAL, {{"Alice", 1}, {"Bob", 1}, {"Chloé", 1}}),
        depense("D3", "Alice", 1, ModeRepartition::EGAL, {{"Bob", 1}, {"Chloé", 1}, {"Dan", 1}}),
        depense("D4", "Bob", 100000000, ModeRepartition::PONDERE,
                {{"Alice", 7}, {"Bob", 3}, {"Chloé", 5}, {"Dan", 11}}),
        depense("D5", "Dan", 9999, ModeRepartition::EGAL, {{"Dan", 1}}),
    };
    for (const auto& d : depenses) {
        groupe.ajouterDepense(d);
        EXPECT_EQ(sommeDesSoldes(calculerBilan(groupe)), 0) << "après " << d.titre;
    }
}

TEST(Bilan, PayeurNonBeneficiaire) {
    Groupe groupe("Week-end BDE");
    groupe.ajouterParticipant("Alice");
    groupe.ajouterParticipant("Bob");
    groupe.ajouterParticipant("Chloé");
    groupe.ajouterDepense(depense("Courses", "Alice", 900, ModeRepartition::EGAL,
                                  {{"Bob", 1}, {"Chloé", 1}}));

    const auto bilan = calculerBilan(groupe);
    verifierLigne(bilan[0], "Alice", 900, 0, 900);
    verifierLigne(bilan[1], "Bob", 0, 450, -450);
    verifierLigne(bilan[2], "Chloé", 0, 450, -450);
}

TEST(Bilan, GroupeSansDepense) {
    Groupe groupe("Week-end BDE");
    groupe.ajouterParticipant("Alice");
    groupe.ajouterParticipant("Bob");

    const auto bilan = calculerBilan(groupe);
    ASSERT_EQ(bilan.size(), 2u);
    verifierLigne(bilan[0], "Alice", 0, 0, 0);
    verifierLigne(bilan[1], "Bob", 0, 0, 0);
}
