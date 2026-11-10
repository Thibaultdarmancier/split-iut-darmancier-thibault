#include "iban.hpp"

#include <cctype>

namespace {

// 1. Normalisation : suppression des espaces + majuscules
std::string normaliser(const std::string& iban) {
    std::string resultat;
    for (unsigned char c : iban) {
        if (c != ' ') {
            resultat += static_cast<char>(std::toupper(c));
        }
    }
    return resultat;
}

// 2. Vérification de structure
bool structureValide(const std::string& iban) {
    if (iban.size() < 15 || iban.size() > 34) {
        return false;
    }
    for (unsigned char c : iban) {
        if (!std::isalnum(c)) {
            return false;
        }
    }
    if (!std::isupper(static_cast<unsigned char>(iban[0])) ||
        !std::isupper(static_cast<unsigned char>(iban[1])) ||
        !std::isdigit(static_cast<unsigned char>(iban[2])) ||
        !std::isdigit(static_cast<unsigned char>(iban[3]))) {
        return false;
    }
    if (iban.compare(0, 2, "FR") == 0 && iban.size() != 27) {
        return false;
    }
    return true;
}

// 3. Modulo 97 calculé chiffre par chiffre : le nombre complet (30+ chiffres)
// dépasse un entier 64 bits, on ne garde donc que le reste à chaque étape.
int modulo97(const std::string& iban) {
    const std::string reordonne = iban.substr(4) + iban.substr(0, 4);
    int reste = 0;
    for (unsigned char c : reordonne) {
        if (std::isdigit(c)) {
            reste = (reste * 10 + (c - '0')) % 97;
        } else {
            const int valeur = c - 'A' + 10;  // A=10 ... Z=35
            reste = (reste * 100 + valeur) % 97;
        }
    }
    return reste;
}

}  // namespace

bool validerIban(const std::string& iban) {
    const std::string normalise = normaliser(iban);
    if (!structureValide(normalise)) {
        return false;
    }
    return modulo97(normalise) == 1;
}