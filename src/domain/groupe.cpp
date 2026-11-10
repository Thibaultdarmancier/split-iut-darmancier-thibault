#include "groupe.hpp"

#include <optional>
#include <stdexcept>

namespace splitiut {

namespace {

bool estEspace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

std::string retirerEspaces(const std::string& texte) {
    std::size_t debut = 0;
    std::size_t fin = texte.size();
    while (debut < fin && estEspace(texte[debut])) {
        ++debut;
    }
    while (fin > debut && estEspace(texte[fin - 1])) {
        --fin;
    }
    return texte.substr(debut, fin - debut);
}

char minusculeAscii(char c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

// Seules les lettres ASCII A-Z sont comparées sans casse ; les autres octets
// (lettres accentuées en UTF-8) doivent être identiques.
bool memeNom(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (minusculeAscii(a[i]) != minusculeAscii(b[i])) {
            return false;
        }
    }
    return true;
}

std::optional<std::size_t> trouverRang(const std::vector<Participant>& participants,
                                       const std::string& nom) {
    const std::string nomNettoye = retirerEspaces(nom);
    for (std::size_t rang = 0; rang < participants.size(); ++rang) {
        if (memeNom(participants[rang].nom, nomNettoye)) {
            return rang;
        }
    }
    return std::nullopt;
}

}  // namespace

Groupe::Groupe(const std::string& nom) : nom_(retirerEspaces(nom)) {
    if (nom_.empty()) {
        throw std::invalid_argument("Nom de groupe invalide : il ne doit pas être vide");
    }
}

void Groupe::ajouterParticipant(const std::string& nom) {
    const std::string nomNettoye = retirerEspaces(nom);
    if (nomNettoye.empty()) {
        throw std::invalid_argument("Nom de participant invalide : il ne doit pas être vide");
    }
    if (const auto rang = trouverRang(participants_, nomNettoye)) {
        throw std::invalid_argument("Participant « " + nomNettoye + " » refusé : « " +
                                    participants_[*rang].nom + " » est déjà dans le groupe");
    }
    participants_.push_back(Participant{nomNettoye});
}

std::size_t Groupe::rangDe(const std::string& nom) const {
    const auto rang = trouverRang(participants_, nom);
    if (!rang) {
        throw std::invalid_argument("Participant « " + nom + " » absent du groupe");
    }
    return *rang;
}

void Groupe::ajouterDepense(const Depense& depense) {
    // Tout est validé sur une copie : le groupe n'est modifié qu'à la toute fin (SPEC §3).
    Depense valide = depense;

    valide.titre = retirerEspaces(depense.titre);
    if (valide.titre.empty()) {
        throw std::invalid_argument("Titre de dépense invalide : il ne doit pas être vide");
    }

    verifierBornesMontant(valide.montant);

    const auto rangPayeur = trouverRang(participants_, depense.payeur);
    if (!rangPayeur) {
        throw std::invalid_argument("Payeur « " + depense.payeur + " » absent du groupe");
    }
    valide.payeur = participants_[*rangPayeur].nom;

    if (valide.beneficiaires.empty()) {
        throw std::invalid_argument("Dépense invalide : il faut au moins un bénéficiaire");
    }

    std::vector<bool> dejaVu(participants_.size(), false);
    for (Beneficiaire& beneficiaire : valide.beneficiaires) {
        const auto rang = trouverRang(participants_, beneficiaire.nom);
        if (!rang) {
            throw std::invalid_argument("Bénéficiaire « " + beneficiaire.nom +
                                        " » absent du groupe");
        }
        if (dejaVu[*rang]) {
            throw std::invalid_argument("Bénéficiaire « " + participants_[*rang].nom +
                                        " » présent plusieurs fois");
        }
        dejaVu[*rang] = true;
        beneficiaire.nom = participants_[*rang].nom;

        if (valide.mode == ModeRepartition::EGAL && beneficiaire.poids != 1) {
            throw std::invalid_argument("Poids de « " + beneficiaire.nom +
                                        " » invalide : il vaut 1 en mode EGAL");
        }
        if (valide.mode == ModeRepartition::PONDERE && beneficiaire.poids <= 0) {
            throw std::invalid_argument("Poids de « " + beneficiaire.nom +
                                        " » invalide : il doit être strictement positif");
        }
    }

    depenses_.push_back(valide);
}

}  // namespace splitiut
