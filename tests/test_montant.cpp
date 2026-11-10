#include <gtest/gtest.h>

#include <stdexcept>

#include "montant.hpp"

using splitiut::Montant;
using splitiut::verifierBornesMontant;

TEST(MontantDepuisTexte, ConvertitEnCentimes) {
    EXPECT_EQ(Montant::depuisTexte("10,00").centimes(), 1000);
    EXPECT_EQ(Montant::depuisTexte("3,34").centimes(), 334);
    EXPECT_EQ(Montant::depuisTexte("0,01").centimes(), 1);
    EXPECT_EQ(Montant::depuisTexte("1000000,00").centimes(), 100000000);
}

TEST(MontantDepuisTexte, AccepteLesZerosDeTete) {
    // "chiffres,2 chiffres" : les zéros de tête restent des chiffres.
    EXPECT_EQ(Montant::depuisTexte("010,00").centimes(), 1000);
}

TEST(MontantDepuisTexte, RejetteLesFormatsInvalides) {
    const char* const saisies[] = {
        "10",      "10,5",      "10.50",    "10,005", "10,00 €", "",
        ",00",     "10,",       "-10,00",   "+10,00", " 10,00",  "10,00 ",
        "1 000,00", "1,0,00",   "10,a0",    "dix,00",
    };
    for (const char* saisie : saisies) {
        EXPECT_THROW(Montant::depuisTexte(saisie), std::invalid_argument) << "saisie : " << saisie;
    }
}

TEST(MontantDepuisTexte, RejetteLesMontantsHorsBornes) {
    EXPECT_THROW(Montant::depuisTexte("0,00"), std::invalid_argument);
    EXPECT_THROW(Montant::depuisTexte("1000000,01"), std::invalid_argument);
    // Saisie qui dépasserait std::int64_t : rejetée sans débordement.
    EXPECT_THROW(Montant::depuisTexte("99999999999999999999999,00"), std::invalid_argument);
}

TEST(MontantVersTexte, FormateAvecVirguleEtDeuxDecimales) {
    EXPECT_EQ(Montant(1000).versTexte(), "10,00");
    EXPECT_EQ(Montant(334).versTexte(), "3,34");
    EXPECT_EQ(Montant(5).versTexte(), "0,05");
    EXPECT_EQ(Montant(0).versTexte(), "0,00");
    EXPECT_EQ(Montant(100000000).versTexte(), "1000000,00");
}

TEST(MontantVersTexte, FormateLesMontantsNegatifs) {
    EXPECT_EQ(Montant(-333).versTexte(), "-3,33");
    EXPECT_EQ(Montant(-5).versTexte(), "-0,05");
}

TEST(Montant, AllerRetourTexte) {
    for (const char* texte : {"0,01", "3,33", "10,00", "1000000,00"}) {
        EXPECT_EQ(Montant::depuisTexte(texte).versTexte(), texte);
    }
}

TEST(Montant, ParDefautVautZero) {
    EXPECT_EQ(Montant().centimes(), 0);
}

TEST(Montant, Egalite) {
    EXPECT_TRUE(Montant(1000) == Montant::depuisTexte("10,00"));
    EXPECT_TRUE(Montant(1000) != Montant(999));
}

TEST(Montant, AdditionEtSoustraction) {
    EXPECT_EQ((Montant(1000) + Montant(334)).centimes(), 1334);
    EXPECT_EQ((Montant(1000) - Montant(1334)).centimes(), -334);

    Montant total;
    total += Montant(500);
    total -= Montant(200);
    EXPECT_EQ(total.centimes(), 300);
}

TEST(VerifierBornesMontant, AccepteDe1CentimeA1MillionDEuros) {
    EXPECT_NO_THROW(verifierBornesMontant(Montant(1)));
    EXPECT_NO_THROW(verifierBornesMontant(Montant(Montant::MAX_CENTIMES)));
}

TEST(VerifierBornesMontant, RejetteHorsBornes) {
    EXPECT_THROW(verifierBornesMontant(Montant(0)), std::invalid_argument);
    EXPECT_THROW(verifierBornesMontant(Montant(-1)), std::invalid_argument);
    EXPECT_THROW(verifierBornesMontant(Montant(Montant::MAX_CENTIMES + 1)), std::invalid_argument);
}
