#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "depense.hpp"

namespace splitiut {

// Participant identifié par son nom (SPEC §1).
// Son rang dans Groupe::participants() est son ordre d'ajout.
struct Participant {
    std::string nom;
};

class Groupe {
public:
    // Le nom est enregistré sans ses espaces de début et de fin.
    // Lève std::invalid_argument s'il est vide après ce retrait.
    explicit Groupe(const std::string& nom);

    const std::string& nom() const { return nom_; }

    // Participants dans l'ordre d'ajout.
    const std::vector<Participant>& participants() const { return participants_; }

    // Dépenses dans l'ordre d'ajout.
    const std::vector<Depense>& depenses() const { return depenses_; }

    // Ajoute un participant en fin de liste, nom sans espaces de début et de fin.
    // Lève std::invalid_argument, groupe inchangé, si le nom est vide ou déjà présent
    // (insensible à la casse ASCII : "alice" = "Alice", mais "Alíce" ≠ "Alice").
    void ajouterParticipant(const std::string& nom);

    // Rang (ordre d'ajout, à partir de 0) du participant, nom comparé comme pour les doublons.
    // Lève std::invalid_argument s'il est absent du groupe.
    std::size_t rangDe(const std::string& nom) const;

    // Valide la dépense (SPEC §3) puis l'ajoute. Titre, payeur et bénéficiaires sont
    // enregistrés normalisés (espaces retirés, noms exacts des participants).
    // Lève std::invalid_argument, groupe inchangé, si la dépense est invalide.
    void ajouterDepense(const Depense& depense);

private:
    std::string nom_;
    std::vector<Participant> participants_;
    std::vector<Depense> depenses_;
};

}  // namespace splitiut
