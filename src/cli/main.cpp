// Interface console SplitIUT : saisie et affichage uniquement, aucune règle métier.
// Toutes les validations sont faites par le domaine (std::invalid_argument) ;
// les erreurs sont attrapées et affichées ici, le programme reste actif.

#include <algorithm>
#include <cstddef>
#include <exception>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "bilan.hpp"
#include "depense.hpp"
#include "groupe.hpp"
#include "montant.hpp"
#include "remboursement.hpp"
#include "repartition.hpp"
#include "stockage_json.hpp"

using splitiut::Beneficiaire;
using splitiut::Depense;
using splitiut::Groupe;
using splitiut::LigneBilan;
using splitiut::ModeRepartition;
using splitiut::Montant;
using splitiut::Participant;
using splitiut::Virement;

namespace {

const std::string FICHIER_PAR_DEFAUT = "groupe.json";

// ---------- Saisie ----------

// Lit une ligne ; false en fin d'entrée. Retire un éventuel '\r' final (Windows).
bool lireLigne(std::string& ligne) {
    if (!std::getline(std::cin, ligne)) {
        return false;
    }
    if (!ligne.empty() && ligne.back() == '\r') {
        ligne.pop_back();
    }
    return true;
}

std::string demander(const std::string& invite) {
    std::cout << invite;
    std::string ligne;
    if (!lireLigne(ligne)) {
        throw std::runtime_error("fin de la saisie");
    }
    return ligne;
}

bool confirmer(const std::string& question) {
    const std::string reponse = demander(question + " (o/n) : ");
    return reponse == "o" || reponse == "O";
}

long long lireNombre(const std::string& texte) {
    try {
        std::size_t lu = 0;
        const long long valeur = std::stoll(texte, &lu);
        if (lu == texte.size()) {
            return valeur;
        }
    } catch (const std::logic_error&) {
        // invalid_argument ou out_of_range : traité ci-dessous.
    }
    throw std::invalid_argument("« " + texte + " » n'est pas un nombre entier valide");
}

int lirePoids(const std::string& texte) {
    const long long poids = lireNombre(texte);
    if (poids < std::numeric_limits<int>::min() || poids > std::numeric_limits<int>::max()) {
        throw std::invalid_argument("Poids « " + texte + " » trop grand");
    }
    return static_cast<int>(poids);
}

// Numéro affiché (à partir de 1) -> participant.
const Participant& choisirParticipant(const Groupe& groupe, const std::string& texte) {
    const long long numero = lireNombre(texte);
    if (numero < 1 || numero > static_cast<long long>(groupe.participants().size())) {
        throw std::invalid_argument("Numéro de participant inconnu : " + texte);
    }
    return groupe.participants()[static_cast<std::size_t>(numero - 1)];
}

std::string demanderChemin() {
    const std::string chemin = demander("Fichier (Entrée = " + FICHIER_PAR_DEFAUT + ") : ");
    return chemin.empty() ? FICHIER_PAR_DEFAUT : chemin;
}

// ---------- Affichage ----------

// Largeur à l'écran d'un texte UTF-8 : un caractère par octet qui n'est pas une suite.
std::size_t largeur(const std::string& texte) {
    std::size_t n = 0;
    for (char c : texte) {
        if ((static_cast<unsigned char>(c) & 0xC0) != 0x80) {
            ++n;
        }
    }
    return n;
}

std::string completerADroite(const std::string& texte, std::size_t taille) {
    const std::size_t l = largeur(texte);
    return l >= taille ? texte : texte + std::string(taille - l, ' ');
}

std::string completerAGauche(const std::string& texte, std::size_t taille) {
    const std::size_t l = largeur(texte);
    return l >= taille ? texte : std::string(taille - l, ' ') + texte;
}

// "+16,66", "−3,33" (signe moins U+2212, comme SPEC §5) ou "0,00".
std::string avecSigne(Montant montant) {
    const std::string texte = montant.versTexte();
    if (montant.centimes() > 0) {
        return "+" + texte;
    }
    if (montant.centimes() < 0) {
        return "−" + texte.substr(1);
    }
    return texte;
}

void afficherParticipants(const Groupe& groupe) {
    for (std::size_t i = 0; i < groupe.participants().size(); ++i) {
        std::cout << "  " << i + 1 << ". " << groupe.participants()[i].nom << '\n';
    }
}

void afficherMenu(const std::optional<Groupe>& groupe) {
    std::cout << "\n=== SplitIUT ===\n";
    if (groupe) {
        std::cout << "Groupe : " << groupe->nom() << " (" << groupe->participants().size()
                  << " participant(s), " << groupe->depenses().size() << " dépense(s))\n";
    } else {
        std::cout << "Aucun groupe\n";
    }
    std::cout << "1. Créer le groupe\n"
                 "2. Ajouter un participant\n"
                 "3. Ajouter une dépense\n"
                 "4. Afficher le bilan\n"
                 "5. Afficher les virements\n"
                 "6. Sauvegarder\n"
                 "7. Charger\n"
                 "0. Quitter\n"
                 "Choix : ";
}

// ---------- Actions ----------

void creerGroupe(std::optional<Groupe>& groupe) {
    if (groupe && !confirmer("Remplacer le groupe « " + groupe->nom() + " » ?")) {
        return;
    }
    Groupe nouveau(demander("Nom du groupe : "));  // si invalide, l'ancien groupe est conservé
    groupe = nouveau;
    std::cout << "Groupe « " << groupe->nom() << " » créé.\n";
}

void ajouterParticipant(Groupe& groupe) {
    groupe.ajouterParticipant(demander("Nom du participant : "));
    std::cout << "Participant « " << groupe.participants().back().nom << " » ajouté.\n";
}

void ajouterDepense(Groupe& groupe) {
    if (groupe.participants().empty()) {
        throw std::invalid_argument("Ajoutez d'abord des participants");
    }

    Depense depense;
    depense.titre = demander("Titre : ");
    depense.montant = Montant::depuisTexte(demander("Montant (ex. 10,00) : "));

    afficherParticipants(groupe);
    depense.payeur = choisirParticipant(groupe, demander("Payeur (numéro) : ")).nom;

    const std::string mode = demander("Répartition (1 = égale, 2 = pondérée) : ");
    if (mode == "1") {
        depense.mode = ModeRepartition::EGAL;
    } else if (mode == "2") {
        depense.mode = ModeRepartition::PONDERE;
    } else {
        throw std::invalid_argument("Répartition inconnue : choisir 1 ou 2");
    }

    std::istringstream numeros(demander("Bénéficiaires (numéros séparés par des espaces) : "));
    std::string numero;
    while (numeros >> numero) {
        Beneficiaire beneficiaire;
        beneficiaire.nom = choisirParticipant(groupe, numero).nom;
        if (depense.mode == ModeRepartition::PONDERE) {
            beneficiaire.poids = lirePoids(demander("Poids de " + beneficiaire.nom + " : "));
        }
        depense.beneficiaires.push_back(beneficiaire);
    }

    groupe.ajouterDepense(depense);

    const Depense& ajoutee = groupe.depenses().back();
    const std::vector<Montant> parts = splitiut::calculerParts(groupe, ajoutee);
    std::cout << "Dépense « " << ajoutee.titre << " » ajoutée. Parts :\n";
    for (std::size_t i = 0; i < parts.size(); ++i) {
        std::cout << "  " << ajoutee.beneficiaires[i].nom << " : " << parts[i].versTexte() << '\n';
    }
}

void afficherBilan(const Groupe& groupe) {
    const std::vector<LigneBilan> bilan = splitiut::calculerBilan(groupe);

    std::size_t colonneNom = largeur("Participant");
    for (const LigneBilan& ligne : bilan) {
        colonneNom = std::max(colonneNom, largeur(ligne.nom));
    }
    const std::size_t colonne = 12;

    std::cout << "\nBilan — " << groupe.nom() << '\n'
              << completerADroite("Participant", colonneNom) << completerAGauche("Payé", colonne)
              << completerAGauche("Consommé", colonne) << completerAGauche("Solde", colonne) << '\n';
    for (const LigneBilan& ligne : bilan) {
        std::cout << completerADroite(ligne.nom, colonneNom)
                  << completerAGauche(ligne.paye.versTexte(), colonne)
                  << completerAGauche(ligne.consomme.versTexte(), colonne)
                  << completerAGauche(avecSigne(ligne.solde), colonne) << '\n';
    }
}

void afficherVirements(const Groupe& groupe) {
    const std::vector<Virement> virements =
        splitiut::calculerVirements(splitiut::calculerBilan(groupe));

    std::cout << "\nVirements — " << groupe.nom() << '\n';
    if (virements.empty()) {
        std::cout << "Aucun virement : tous les soldes sont à 0,00.\n";
    }
    for (const Virement& virement : virements) {
        std::cout << virement.debiteur << " → " << virement.crediteur << " : "
                  << virement.montant.versTexte() << '\n';
    }
}

void sauvegarder(const Groupe& groupe) {
    const std::string chemin = demanderChemin();
    splitiut::sauvegarderGroupe(groupe, chemin);
    std::cout << "Groupe sauvegardé dans « " << chemin << " ».\n";
}

void charger(std::optional<Groupe>& groupe) {
    if (groupe && !confirmer("Remplacer le groupe « " + groupe->nom() + " » ?")) {
        return;
    }
    const std::string chemin = demanderChemin();
    Groupe charge = splitiut::chargerGroupe(chemin);  // si invalide, l'ancien groupe est conservé
    groupe = charge;
    std::cout << "Groupe « " << groupe->nom() << " » chargé depuis « " << chemin << " ».\n";
}

}  // namespace

int main() {
#ifdef _WIN32
    // Console en UTF-8 pour afficher et saisir « Chloé ».
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::optional<Groupe> groupe;
    while (true) {
        afficherMenu(groupe);
        std::string choix;
        if (!lireLigne(choix) || choix == "0") {
            break;
        }

        const bool besoinGroupe =
            choix == "2" || choix == "3" || choix == "4" || choix == "5" || choix == "6";
        try {
            if (choix == "1") {
                creerGroupe(groupe);
            } else if (choix == "7") {
                charger(groupe);
            } else if (!besoinGroupe) {
                std::cout << "Choix inconnu.\n";
            } else if (!groupe) {
                std::cout << "Aucun groupe : créez (1) ou chargez (7) un groupe d'abord.\n";
            } else if (choix == "2") {
                ajouterParticipant(*groupe);
            } else if (choix == "3") {
                ajouterDepense(*groupe);
            } else if (choix == "4") {
                afficherBilan(*groupe);
            } else if (choix == "5") {
                afficherVirements(*groupe);
            } else {
                sauvegarder(*groupe);
            }
        } catch (const std::exception& erreur) {
            std::cout << "Erreur : " << erreur.what() << '\n';
        }
    }

    std::cout << "Au revoir.\n";
    return 0;
}
