#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

#include "depense.hpp"
#include "exemples_spec.hpp"
#include "groupe.hpp"

using exemples::depense;
using splitiut::Depense;
using splitiut::Groupe;
using splitiut::ModeRepartition;
using splitiut::Montant;

namespace {

Groupe groupeABC() {
    Groupe groupe("Week-end BDE");
    groupe.ajouterParticipant("Alice");
    groupe.ajouterParticipant("Bob");
    groupe.ajouterParticipant("Chloé");
    return groupe;
}

Depense pizzas() {
    return depense("Pizzas", "Bob", 1000, ModeRepartition::EGAL,
                   {{"Alice", 1}, {"Bob", 1}, {"Chloé", 1}});
}

// La dépense est rejetée et le groupe reste sans dépense.
void verifierRejet(const Depense& invalide) {
    Groupe groupe = groupeABC();
    EXPECT_THROW(groupe.ajouterDepense(invalide), std::invalid_argument);
    EXPECT_TRUE(groupe.depenses().empty());
}

}  // namespace

TEST(Depense, AjoutValide) {
    Groupe groupe = groupeABC();
    groupe.ajouterDepense(pizzas());

    ASSERT_EQ(groupe.depenses().size(), 1u);
    const Depense& ajoutee = groupe.depenses()[0];
    EXPECT_EQ(ajoutee.titre, "Pizzas");
    EXPECT_EQ(ajoutee.montant.centimes(), 1000);
    EXPECT_EQ(ajoutee.payeur, "Bob");
    EXPECT_EQ(ajoutee.mode, ModeRepartition::EGAL);
    ASSERT_EQ(ajoutee.beneficiaires.size(), 3u);
}

TEST(Depense, AjoutNormaliseTitreEtNoms) {
    Groupe groupe = groupeABC();
    groupe.ajouterDepense(depense("  Pizzas  ", " bob ", 1000, ModeRepartition::EGAL,
                                  {{"ALICE", 1}, {"chloé", 1}}));

    const Depense& ajoutee = groupe.depenses()[0];
    EXPECT_EQ(ajoutee.titre, "Pizzas");
    EXPECT_EQ(ajoutee.payeur, "Bob");
    EXPECT_EQ(ajoutee.beneficiaires[0].nom, "Alice");
    EXPECT_EQ(ajoutee.beneficiaires[1].nom, "Chloé");
}

TEST(Depense, PayeurPeutNePasEtreBeneficiaire) {
    Groupe groupe = groupeABC();
    groupe.ajouterDepense(depense("Courses", "Alice", 900, ModeRepartition::EGAL,
                                  {{"Bob", 1}, {"Chloé", 1}}));
    EXPECT_EQ(groupe.depenses().size(), 1u);
}

TEST(Depense, TitreVideRejete) {
    Depense d = pizzas();
    d.titre = "";
    verifierRejet(d);
    d.titre = "   ";
    verifierRejet(d);
}

TEST(Depense, MontantHorsBornesRejete) {
    Depense d = pizzas();
    d.montant = Montant(0);
    verifierRejet(d);
    d.montant = Montant(-100);
    verifierRejet(d);
    d.montant = Montant(Montant::MAX_CENTIMES + 1);
    verifierRejet(d);
}

TEST(Depense, PayeurAbsentRejete) {
    Depense d = pizzas();
    d.payeur = "Zoé";
    verifierRejet(d);
}

TEST(Depense, BeneficiairesVidesRejetes) {
    Depense d = pizzas();
    d.beneficiaires.clear();
    verifierRejet(d);
}

TEST(Depense, BeneficiaireAbsentRejete) {
    Depense d = pizzas();
    d.beneficiaires.push_back({"Zoé", 1});
    verifierRejet(d);
}

TEST(Depense, BeneficiaireEnDoubleRejete) {
    Depense d = pizzas();
    d.beneficiaires.push_back({"alice", 1});
    verifierRejet(d);
}

TEST(Depense, PoidsNulOuNegatifRejeteEnPondere) {
    Depense d = depense("Courses", "Alice", 1000, ModeRepartition::PONDERE, {{"Alice", 2}, {"Bob", 0}});
    verifierRejet(d);
    d.beneficiaires[1].poids = -1;
    verifierRejet(d);
}

TEST(Depense, PoidsDifferentDe1RejeteEnEgal) {
    Depense d = pizzas();
    d.beneficiaires[0].poids = 2;
    verifierRejet(d);
}

TEST(Depense, RejetSurUneDepenseConserveLesPrecedentes) {
    Groupe groupe = groupeABC();
    groupe.ajouterDepense(pizzas());

    Depense invalide = pizzas();
    invalide.payeur = "Zoé";
    EXPECT_THROW(groupe.ajouterDepense(invalide), std::invalid_argument);

    ASSERT_EQ(groupe.depenses().size(), 1u);
    EXPECT_EQ(groupe.depenses()[0].payeur, "Bob");
}

TEST(Groupe, RangDe) {
    const Groupe groupe = groupeABC();
    EXPECT_EQ(groupe.rangDe("Alice"), 0u);
    EXPECT_EQ(groupe.rangDe("  chloé "), 2u);
    EXPECT_THROW(groupe.rangDe("Zoé"), std::invalid_argument);
}

TEST(ModeDeRepartition, ConversionTexte) {
    EXPECT_EQ(splitiut::versTexte(ModeRepartition::EGAL), "EGAL");
    EXPECT_EQ(splitiut::versTexte(ModeRepartition::PONDERE), "PONDERE");
    EXPECT_EQ(splitiut::modeDepuisTexte("EGAL"), ModeRepartition::EGAL);
    EXPECT_EQ(splitiut::modeDepuisTexte("PONDERE"), ModeRepartition::PONDERE);
    EXPECT_THROW(splitiut::modeDepuisTexte("egal"), std::invalid_argument);
    EXPECT_THROW(splitiut::modeDepuisTexte(""), std::invalid_argument);
}
