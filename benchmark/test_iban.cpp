#include <gtest/gtest.h>

#include "iban.hpp"

// (1) IBAN français valide
TEST(Iban, FrancaisValide) {
    EXPECT_TRUE(validerIban("FR76 3000 6000 0112 3456 7890 189"));
}

// (2) Le même sans espaces et en minuscules
TEST(Iban, SansEspacesEtMinuscules) {
    EXPECT_TRUE(validerIban("fr7630006000011234567890189"));
}

// (3) Clé de contrôle invalide (76 -> 77)
TEST(Iban, CleInvalide) {
    EXPECT_FALSE(validerIban("FR77 3000 6000 0112 3456 7890 189"));
}

// (4) IBAN FR de mauvaise longueur (26 caractères au lieu de 27)
TEST(Iban, FrancaisMauvaiseLongueur) {
    EXPECT_FALSE(validerIban("FR76 3000 6000 0112 3456 7890 18"));
}

// (5) Chaîne contenant un caractère spécial
TEST(Iban, CaractereSpecial) {
    EXPECT_FALSE(validerIban("FR76 3000 6000 0112 3456 7890 18!"));
}