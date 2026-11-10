#include "montant.hpp"

#include <stdexcept>

namespace splitiut {

namespace {

bool estChiffre(char c) {
    return c >= '0' && c <= '9';
}

[[noreturn]] void rejeterFormat(const std::string& texte) {
    throw std::invalid_argument("Montant invalide « " + texte +
                                " » : format attendu chiffres,2 chiffres (ex. 10,00)");
}

}  // namespace

void verifierBornesMontant(Montant montant) {
    if (montant.centimes() <= 0) {
        throw std::invalid_argument("Montant invalide : il doit être strictement positif");
    }
    if (montant.centimes() > Montant::MAX_CENTIMES) {
        throw std::invalid_argument("Montant invalide : il ne doit pas dépasser 1000000,00 €");
    }
}

Montant Montant::depuisTexte(const std::string& texte) {
    // 1. Format : au moins un chiffre, une virgule, exactement deux chiffres.
    const std::size_t virgule = texte.find(',');
    if (virgule == std::string::npos || virgule == 0 || texte.size() != virgule + 3) {
        rejeterFormat(texte);
    }
    for (std::size_t i = 0; i < texte.size(); ++i) {
        if (i != virgule && !estChiffre(texte[i])) {
            rejeterFormat(texte);
        }
    }

    // 2. Conversion. Une fois la borne dépassée, on n'accumule plus :
    //    une saisie très longue ne peut pas faire déborder l'entier.
    std::int64_t euros = 0;
    for (std::size_t i = 0; i < virgule; ++i) {
        if (euros <= MAX_CENTIMES / 100) {
            euros = euros * 10 + (texte[i] - '0');
        }
    }
    const std::int64_t centimes = (texte[virgule + 1] - '0') * 10 + (texte[virgule + 2] - '0');
    const Montant montant(euros * 100 + centimes);

    // 3. Bornes.
    verifierBornesMontant(montant);
    return montant;
}

std::string Montant::versTexte() const {
    const bool negatif = centimes_ < 0;
    // Valeur absolue en non signé : pas de débordement, même pour INT64_MIN.
    const std::uint64_t valeur = negatif ? 0 - static_cast<std::uint64_t>(centimes_)
                                         : static_cast<std::uint64_t>(centimes_);
    const std::uint64_t centimes = valeur % 100;

    std::string texte = negatif ? "-" : "";
    texte += std::to_string(valeur / 100);
    texte += ',';
    if (centimes < 10) {
        texte += '0';
    }
    texte += std::to_string(centimes);
    return texte;
}

}  // namespace splitiut
