#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

#include "bilan.hpp"
#include "exemples_spec.hpp"
#include "remboursement.hpp"
#include "stockage_json.hpp"

using splitiut::calculerBilan;
using splitiut::calculerVirements;
using splitiut::chargerGroupe;
using splitiut::Groupe;
using splitiut::sauvegarderGroupe;

namespace {

// Fichier dans le dossier courant (ctest lance les tests depuis build/).
// Pas de <filesystem> : il ne compile pas avec MinGW GCC 8 sous Windows.
std::string cheminTemporaire(const std::string& nom) {
    return "splitiut_test_" + nom + ".json";
}

void ecrire(const std::string& chemin, const std::string& contenu) {
    std::ofstream fichier(chemin, std::ios::binary);
    fichier << contenu;
}

std::string lire(const std::string& chemin) {
    std::ifstream fichier(chemin, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(fichier), std::istreambuf_iterator<char>());
}

// Groupe JSON valide sauf pour le fragment remplacé.
std::string groupeJson(const std::string& participants, const std::string& depenses) {
    return R"({"nom": "Test", "participants": )" + participants + R"(, "depenses": )" + depenses + "}";
}

std::string depenseJson(const std::string& montant, const std::string& payeur = "\"Bob\"",
                        const std::string& mode = "\"EGAL\"") {
    return R"([{"titre": "Pizzas", "montant": )" + montant + R"(, "payeur": )" + payeur +
           R"(, "mode": )" + mode + R"(, "beneficiaires": [{"nom": "Alice", "poids": 1}]}])";
}

const std::string PARTICIPANTS = R"(["Alice", "Bob"])";

void verifierRejet(const std::string& nom, const std::string& contenu) {
    const std::string chemin = cheminTemporaire(nom);
    ecrire(chemin, contenu);
    EXPECT_THROW(chargerGroupe(chemin), std::invalid_argument) << contenu;
    std::remove(chemin.c_str());
}

}  // namespace

TEST(Stockage, SauvegardePuisChargementMemesSoldesEtVirements) {
    const std::string chemin = cheminTemporaire("aller_retour");
    const Groupe original = exemples::exemple2();
    sauvegarderGroupe(original, chemin);
    const Groupe charge = chargerGroupe(chemin);
    std::remove(chemin.c_str());

    EXPECT_EQ(charge.nom(), original.nom());
    ASSERT_EQ(charge.depenses().size(), original.depenses().size());

    const auto bilanOriginal = calculerBilan(original);
    const auto bilanCharge = calculerBilan(charge);
    ASSERT_EQ(bilanCharge.size(), bilanOriginal.size());
    for (std::size_t i = 0; i < bilanOriginal.size(); ++i) {
        EXPECT_EQ(bilanCharge[i].nom, bilanOriginal[i].nom);
        EXPECT_EQ(bilanCharge[i].solde, bilanOriginal[i].solde) << bilanOriginal[i].nom;
    }

    const auto virementsOriginal = calculerVirements(bilanOriginal);
    const auto virementsCharge = calculerVirements(bilanCharge);
    ASSERT_EQ(virementsCharge.size(), virementsOriginal.size());
    for (std::size_t i = 0; i < virementsOriginal.size(); ++i) {
        EXPECT_EQ(virementsCharge[i].debiteur, virementsOriginal[i].debiteur);
        EXPECT_EQ(virementsCharge[i].crediteur, virementsOriginal[i].crediteur);
        EXPECT_EQ(virementsCharge[i].montant, virementsOriginal[i].montant);
    }
}

TEST(Stockage, OrdreDesParticipantsConserve) {
    Groupe groupe("Ordre");
    for (const char* nom : {"Dan", "Chloé", "Alice", "Bob"}) {
        groupe.ajouterParticipant(nom);
    }
    const std::string chemin = cheminTemporaire("ordre");
    sauvegarderGroupe(groupe, chemin);
    const Groupe charge = chargerGroupe(chemin);
    std::remove(chemin.c_str());

    ASSERT_EQ(charge.participants().size(), 4u);
    EXPECT_EQ(charge.participants()[0].nom, "Dan");
    EXPECT_EQ(charge.participants()[1].nom, "Chloé");
    EXPECT_EQ(charge.participants()[2].nom, "Alice");
    EXPECT_EQ(charge.participants()[3].nom, "Bob");
}

TEST(Stockage, MontantEcritEnCentimesEntiers) {
    const std::string chemin = cheminTemporaire("centimes");
    sauvegarderGroupe(exemples::exemple1(), chemin);
    const std::string contenu = lire(chemin);
    std::remove(chemin.c_str());

    EXPECT_NE(contenu.find("\"montant\": 3000"), std::string::npos) << contenu;
    EXPECT_NE(contenu.find("\"Chloé\""), std::string::npos) << contenu;  // UTF-8 brut
}

TEST(Stockage, ChargeUnFichierEcritALaMain) {
    const std::string chemin = cheminTemporaire("manuel");
    ecrire(chemin, groupeJson(PARTICIPANTS, depenseJson("1000")));
    const Groupe groupe = chargerGroupe(chemin);
    std::remove(chemin.c_str());

    ASSERT_EQ(groupe.depenses().size(), 1u);
    EXPECT_EQ(groupe.depenses()[0].montant.centimes(), 1000);
}

TEST(Stockage, MontantNonEntierRejete) {
    verifierRejet("montant_decimal", groupeJson(PARTICIPANTS, depenseJson("10.5")));
    verifierRejet("montant_flottant", groupeJson(PARTICIPANTS, depenseJson("10.0")));
    verifierRejet("montant_texte", groupeJson(PARTICIPANTS, depenseJson("\"10,00\"")));
}

TEST(Stockage, MontantHorsBornesRejete) {
    verifierRejet("montant_nul", groupeJson(PARTICIPANTS, depenseJson("0")));
    verifierRejet("montant_trop_grand", groupeJson(PARTICIPANTS, depenseJson("100000001")));
    verifierRejet("montant_enorme", groupeJson(PARTICIPANTS, depenseJson("18446744073709551615")));
}

TEST(Stockage, ParticipantEnDoubleRejete) {
    verifierRejet("doublon", groupeJson(R"(["Alice", "Bob", "alice"])", "[]"));
}

TEST(Stockage, PayeurInconnuRejete) {
    verifierRejet("payeur", groupeJson(PARTICIPANTS, depenseJson("1000", "\"Zoé\"")));
}

TEST(Stockage, ModeInconnuRejete) {
    verifierRejet("mode", groupeJson(PARTICIPANTS, depenseJson("1000", "\"Bob\"", "\"egal\"")));
}

TEST(Stockage, StructureInvalideRejetee) {
    verifierRejet("syntaxe", "{ pas du json");
    verifierRejet("racine", "[]");
    verifierRejet("champ_manquant", R"({"nom": "Test", "participants": []})");
    verifierRejet("participants_objet", groupeJson("{}", "[]"));
    verifierRejet("poids_decimal",
                  groupeJson(PARTICIPANTS, R"([{"titre": "Pizzas", "montant": 1000, "payeur": "Bob",
                      "mode": "PONDERE", "beneficiaires": [{"nom": "Alice", "poids": 1.5}]}])"));
}

TEST(Stockage, FichierAbsentErreurExplicite) {
    const std::string chemin = cheminTemporaire("inexistant");
    std::remove(chemin.c_str());
    try {
        chargerGroupe(chemin);
        FAIL() << "aucune exception levée";
    } catch (const std::runtime_error& erreur) {
        EXPECT_NE(std::string(erreur.what()).find(chemin), std::string::npos) << erreur.what();
    }
}