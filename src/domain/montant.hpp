#pragma once

#include <cstdint>
#include <string>

namespace splitiut {

// Montant exprimé en centimes entiers (SPEC §1). Aucun flottant.
class Montant {
public:
    // Borne haute d'un montant de dépense : 1 000 000,00 € (SPEC §3).
    static constexpr std::int64_t MAX_CENTIMES = 100'000'000;

    Montant() = default;
    explicit Montant(std::int64_t centimes) : centimes_(centimes) {}

    std::int64_t centimes() const { return centimes_; }

    // Lit une saisie au format "chiffres,2 chiffres" (ex. "10,00" -> 1000 centimes).
    // Lève std::invalid_argument si le format est invalide, si le montant est <= 0
    // ou s'il dépasse 1 000 000,00 €.
    static Montant depuisTexte(const std::string& texte);

    // Écrit le montant au format "10,00" ("-3,33" s'il est négatif).
    std::string versTexte() const;

    bool operator==(Montant autre) const { return centimes_ == autre.centimes_; }
    bool operator!=(Montant autre) const { return centimes_ != autre.centimes_; }
    Montant operator+(Montant autre) const { return Montant(centimes_ + autre.centimes_); }
    Montant operator-(Montant autre) const { return Montant(centimes_ - autre.centimes_); }
    Montant& operator+=(Montant autre) {
        centimes_ += autre.centimes_;
        return *this;
    }
    Montant& operator-=(Montant autre) {
        centimes_ -= autre.centimes_;
        return *this;
    }

private:
    std::int64_t centimes_ = 0;
};

// Vérifie les bornes d'un montant de dépense : 0 < montant <= 1 000 000,00 €.
// Lève std::invalid_argument sinon.
void verifierBornesMontant(Montant montant);

}  // namespace splitiut
