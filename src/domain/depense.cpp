#include "depense.hpp"

#include <stdexcept>

namespace splitiut {

std::string versTexte(ModeRepartition mode) {
    return mode == ModeRepartition::EGAL ? "EGAL" : "PONDERE";
}

ModeRepartition modeDepuisTexte(const std::string& texte) {
    if (texte == "EGAL") {
        return ModeRepartition::EGAL;
    }
    if (texte == "PONDERE") {
        return ModeRepartition::PONDERE;
    }
    throw std::invalid_argument("Mode de répartition « " + texte +
                                " » inconnu : attendu EGAL ou PONDERE");
}

}  // namespace splitiut
