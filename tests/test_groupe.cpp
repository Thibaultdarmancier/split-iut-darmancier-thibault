#include <gtest/gtest.h>

#include <stdexcept>

#include "groupe.hpp"

using splitiut::Groupe;

TEST(Groupe, CreationRetireLesEspaces) {
    const Groupe groupe("  Week-end BDE  ");
    EXPECT_EQ(groupe.nom(), "Week-end BDE");
    EXPECT_TRUE(groupe.participants().empty());
}

TEST(Groupe, NomDeGroupeVideRejete) {
    EXPECT_THROW(Groupe(""), std::invalid_argument);
    EXPECT_THROW(Groupe("   "), std::invalid_argument);
    EXPECT_THROW(Groupe(" \t\n"), std::invalid_argument);
}

TEST(Groupe, AjoutConserveLOrdre) {
    Groupe groupe("Week-end BDE");
    groupe.ajouterParticipant("Chloé");
    groupe.ajouterParticipant("Alice");
    groupe.ajouterParticipant("Bob");

    const auto& participants = groupe.participants();
    ASSERT_EQ(participants.size(), 3u);
    EXPECT_EQ(participants[0].nom, "Chloé");
    EXPECT_EQ(participants[1].nom, "Alice");
    EXPECT_EQ(participants[2].nom, "Bob");
}

TEST(Groupe, AjoutRetireLesEspaces) {
    Groupe groupe("Week-end BDE");
    groupe.ajouterParticipant("  Bob\t");
    ASSERT_EQ(groupe.participants().size(), 1u);
    EXPECT_EQ(groupe.participants()[0].nom, "Bob");
}

TEST(Groupe, NomDeParticipantVideRejeteEtGroupeInchange) {
    Groupe groupe("Week-end BDE");
    groupe.ajouterParticipant("Alice");

    EXPECT_THROW(groupe.ajouterParticipant(""), std::invalid_argument);
    EXPECT_THROW(groupe.ajouterParticipant("   "), std::invalid_argument);

    ASSERT_EQ(groupe.participants().size(), 1u);
    EXPECT_EQ(groupe.participants()[0].nom, "Alice");
}

TEST(Groupe, DoublonInsensibleALaCasseAsciiRejeteEtGroupeInchange) {
    Groupe groupe("Week-end BDE");
    groupe.ajouterParticipant("Alice");

    EXPECT_THROW(groupe.ajouterParticipant("Alice"), std::invalid_argument);
    EXPECT_THROW(groupe.ajouterParticipant("alice"), std::invalid_argument);
    EXPECT_THROW(groupe.ajouterParticipant("ALICE"), std::invalid_argument);
    EXPECT_THROW(groupe.ajouterParticipant("  alice  "), std::invalid_argument);

    ASSERT_EQ(groupe.participants().size(), 1u);
    EXPECT_EQ(groupe.participants()[0].nom, "Alice");
}

TEST(Groupe, AccentsDistincts) {
    Groupe groupe("Week-end BDE");
    groupe.ajouterParticipant("Alice");
    groupe.ajouterParticipant("Alíce");

    ASSERT_EQ(groupe.participants().size(), 2u);
    EXPECT_EQ(groupe.participants()[1].nom, "Alíce");
}

TEST(Groupe, CasseIgnoreeSeulementSurLesLettresAscii) {
    Groupe groupe("Week-end BDE");
    groupe.ajouterParticipant("Chloé");

    // C/c est ASCII et é est identique : doublon.
    EXPECT_THROW(groupe.ajouterParticipant("chloé"), std::invalid_argument);
    // É/é n'est pas ASCII : SPEC §3 les considère comme distincts.
    groupe.ajouterParticipant("CHLOÉ");

    ASSERT_EQ(groupe.participants().size(), 2u);
    EXPECT_EQ(groupe.participants()[1].nom, "CHLOÉ");
}
