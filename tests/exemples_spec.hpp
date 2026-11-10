#pragma once

// Données partagées par les tests : exemples chiffrés de SPEC.md §5.

#include <cstdint>
#include <string>
#include <vector>

#include "depense.hpp"
#include "groupe.hpp"

namespace exemples {

inline splitiut::Depense depense(const std::string& titre, const std::string& payeur,
                                 std::int64_t centimes, splitiut::ModeRepartition mode,
                                 const std::vector<splitiut::Beneficiaire>& beneficiaires) {
    splitiut::Depense resultat;
    resultat.titre = titre;
    resultat.montant = splitiut::Montant(centimes);
    resultat.payeur = payeur;
    resultat.mode = mode;
    resultat.beneficiaires = beneficiaires;
    return resultat;
}

// Exemple 1 : Alice, Bob, Chloé ; D1 Alice 30,00 égal ; D2 Bob 10,00 égal.
inline splitiut::Groupe exemple1() {
    using splitiut::ModeRepartition;
    splitiut::Groupe groupe("Exemple 1");
    groupe.ajouterParticipant("Alice");
    groupe.ajouterParticipant("Bob");
    groupe.ajouterParticipant("Chloé");
    groupe.ajouterDepense(depense("D1", "Alice", 3000, ModeRepartition::EGAL,
                                  {{"Alice", 1}, {"Bob", 1}, {"Chloé", 1}}));
    groupe.ajouterDepense(depense("D2", "Bob", 1000, ModeRepartition::EGAL,
                                  {{"Alice", 1}, {"Bob", 1}, {"Chloé", 1}}));
    return groupe;
}

// Exemple 2 : Alice, Bob, Chloé, Dan ; D1 Chloé 100,00 pondéré A=2 B=1 D=3 ;
// D2 Dan 70,00 égal entre Alice, Bob, Chloé.
inline splitiut::Groupe exemple2() {
    using splitiut::ModeRepartition;
    splitiut::Groupe groupe("Exemple 2");
    groupe.ajouterParticipant("Alice");
    groupe.ajouterParticipant("Bob");
    groupe.ajouterParticipant("Chloé");
    groupe.ajouterParticipant("Dan");
    groupe.ajouterDepense(depense("D1", "Chloé", 10000, ModeRepartition::PONDERE,
                                  {{"Alice", 2}, {"Bob", 1}, {"Dan", 3}}));
    groupe.ajouterDepense(depense("D2", "Dan", 7000, ModeRepartition::EGAL,
                                  {{"Alice", 1}, {"Bob", 1}, {"Chloé", 1}}));
    return groupe;
}

}  // namespace exemples
