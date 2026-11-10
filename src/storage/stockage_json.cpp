#include "stockage_json.hpp"

#include <climits>
#include <cstdint>
#include <fstream>
#include <limits>
#include <stdexcept>

#include <nlohmann/json.hpp>

#include "depense.hpp"

namespace splitiut {

namespace {

// ordered_json : les clés sont écrites dans l'ordre d'AGENTS.md (nom, participants, depenses).
using Json = nlohmann::ordered_json;

[[noreturn]] void rejeter(const std::string& message) {
    throw std::invalid_argument(message);
}

const Json& champ(const Json& objet, const char* nom, const std::string& contexte) {
    if (!objet.is_object()) {
        rejeter(contexte + " doit être un objet");
    }
    const auto it = objet.find(nom);
    if (it == objet.end()) {
        rejeter("champ « " + std::string(nom) + " » manquant dans " + contexte);
    }
    return *it;
}

std::string lireTexte(const Json& valeur, const std::string& contexte) {
    if (!valeur.is_string()) {
        rejeter(contexte + " doit être une chaîne");
    }
    return valeur.get<std::string>();
}

// Refuse 10.5, 10.0 (flottants) et "10,00" (chaîne) : seul un entier JSON est accepté.
std::int64_t lireEntier(const Json& valeur, const std::string& contexte) {
    if (!valeur.is_number_integer()) {
        rejeter(contexte + " doit être un nombre entier");
    }
    if (valeur.is_number_unsigned() &&
        valeur.get<std::uint64_t>() >
            static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        rejeter(contexte + " est trop grand");
    }
    return valeur.get<std::int64_t>();
}

void verifierTableau(const Json& valeur, const std::string& contexte) {
    if (!valeur.is_array()) {
        rejeter(contexte + " doit être un tableau");
    }
}

Beneficiaire lireBeneficiaire(const Json& objet, const std::string& contexte) {
    Beneficiaire beneficiaire;
    beneficiaire.nom = lireTexte(champ(objet, "nom", contexte), contexte + ", « nom »");
    const std::int64_t poids = lireEntier(champ(objet, "poids", contexte), contexte + ", « poids »");
    if (poids < INT_MIN || poids > INT_MAX) {
        rejeter(contexte + ", « poids » hors bornes");
    }
    beneficiaire.poids = static_cast<int>(poids);  // poids <= 0 : rejeté par le domaine
    return beneficiaire;
}

Depense lireDepense(const Json& objet, const std::string& contexte) {
    Depense depense;
    depense.titre = lireTexte(champ(objet, "titre", contexte), contexte + ", « titre »");
    depense.montant = Montant(lireEntier(champ(objet, "montant", contexte), contexte + ", « montant »"));
    depense.payeur = lireTexte(champ(objet, "payeur", contexte), contexte + ", « payeur »");
    depense.mode = modeDepuisTexte(lireTexte(champ(objet, "mode", contexte), contexte + ", « mode »"));

    const Json& beneficiaires = champ(objet, "beneficiaires", contexte);
    verifierTableau(beneficiaires, contexte + ", « beneficiaires »");
    std::size_t numero = 0;
    for (const Json& beneficiaire : beneficiaires) {
        depense.beneficiaires.push_back(
            lireBeneficiaire(beneficiaire, contexte + ", bénéficiaire n°" + std::to_string(++numero)));
    }
    return depense;
}

Groupe lireGroupe(const Json& racine) {
    const std::string laRacine = "la racine";
    Groupe groupe(lireTexte(champ(racine, "nom", laRacine), "« nom »"));

    const Json& participants = champ(racine, "participants", laRacine);
    verifierTableau(participants, "« participants »");
    for (const Json& nom : participants) {
        groupe.ajouterParticipant(lireTexte(nom, "un participant"));
    }

    const Json& depenses = champ(racine, "depenses", laRacine);
    verifierTableau(depenses, "« depenses »");
    std::size_t numero = 0;
    for (const Json& objet : depenses) {
        const std::string contexte = "dépense n°" + std::to_string(++numero);
        try {
            groupe.ajouterDepense(lireDepense(objet, contexte));
        } catch (const std::invalid_argument& erreur) {
            throw std::invalid_argument(contexte + " : " + erreur.what());
        }
    }
    return groupe;
}

}  // namespace

void sauvegarderGroupe(const Groupe& groupe, const std::string& chemin) {
    Json racine = Json::object();
    racine["nom"] = groupe.nom();

    racine["participants"] = Json::array();
    for (const Participant& participant : groupe.participants()) {
        racine["participants"].push_back(participant.nom);
    }

    racine["depenses"] = Json::array();
    for (const Depense& depense : groupe.depenses()) {
        Json beneficiaires = Json::array();
        for (const Beneficiaire& beneficiaire : depense.beneficiaires) {
            Json objet = Json::object();
            objet["nom"] = beneficiaire.nom;
            objet["poids"] = beneficiaire.poids;
            beneficiaires.push_back(objet);
        }
        Json objet = Json::object();
        objet["titre"] = depense.titre;
        objet["montant"] = depense.montant.centimes();
        objet["payeur"] = depense.payeur;
        objet["mode"] = versTexte(depense.mode);
        objet["beneficiaires"] = beneficiaires;
        racine["depenses"].push_back(objet);
    }

    std::string texte;
    try {
        texte = racine.dump(2);
    } catch (const nlohmann::json::exception& erreur) {
        // Ex. : nom saisi dans un encodage autre que UTF-8.
        throw std::runtime_error(std::string("Sauvegarde impossible : ") + erreur.what());
    }

    std::ofstream fichier(chemin, std::ios::binary);
    if (!fichier) {
        throw std::runtime_error("Impossible d'écrire le fichier « " + chemin + " »");
    }
    fichier << texte << '\n';
    if (!fichier) {
        throw std::runtime_error("Erreur pendant l'écriture du fichier « " + chemin + " »");
    }
}

Groupe chargerGroupe(const std::string& chemin) {
    std::ifstream fichier(chemin, std::ios::binary);
    if (!fichier) {
        throw std::runtime_error("Impossible d'ouvrir le fichier « " + chemin +
                                 " » : fichier absent ou illisible");
    }

    try {
        Json racine;
        try {
            racine = Json::parse(fichier);
        } catch (const nlohmann::json::parse_error& erreur) {
            rejeter(std::string("JSON mal formé (") + erreur.what() + ")");
        }
        return lireGroupe(racine);
    } catch (const std::invalid_argument& erreur) {
        throw std::invalid_argument("Fichier « " + chemin + " » invalide : " + erreur.what());
    }
}

}  // namespace splitiut
