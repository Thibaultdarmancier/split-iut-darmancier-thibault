#pragma once

#include <string>

// Valide un IBAN selon ISO 13616 (normalisation, structure, modulo 97).
bool validerIban(const std::string& iban);