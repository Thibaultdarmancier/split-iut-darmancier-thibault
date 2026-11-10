#pragma once

#include <string>

#include "groupe.hpp"

namespace splitiut {

// Écrit le groupe au format JSON d'AGENTS.md (montants en centimes entiers).
// Lève std::runtime_error si le fichier ne peut pas être écrit.
void sauvegarderGroupe(const Groupe& groupe, const std::string& chemin);

// Lit un groupe JSON et le reconstruit via les fonctions du domaine :
// un fichier invalide est rejeté par les mêmes validations que la saisie.
// Lève std::runtime_error si le fichier est absent ou illisible,
// std::invalid_argument si son contenu est invalide.
Groupe chargerGroupe(const std::string& chemin);

}  // namespace splitiut
