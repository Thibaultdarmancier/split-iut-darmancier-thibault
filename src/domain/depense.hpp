#pragma once

#include <string>
#include <vector>

#include "montant.hpp"

namespace splitiut {

enum class ModeRepartition { EGAL, PONDERE };

// Bénéficiaire d'une dépense : nom d'un participant du groupe et poids (1 en mode EGAL).
struct Beneficiaire {
    std::string nom;
    int poids = 1;
};

// Dépense (SPEC §1). Validée et normalisée par Groupe::ajouterDepense.
struct Depense {
    std::string titre;
    Montant montant;
    std::string payeur;
    ModeRepartition mode = ModeRepartition::EGAL;
    std::vector<Beneficiaire> beneficiaires;
};

// "EGAL" / "PONDERE".
std::string versTexte(ModeRepartition mode);

// Lit "EGAL" ou "PONDERE" (casse exacte). Lève std::invalid_argument sinon.
ModeRepartition modeDepuisTexte(const std::string& texte);

}  // namespace splitiut
