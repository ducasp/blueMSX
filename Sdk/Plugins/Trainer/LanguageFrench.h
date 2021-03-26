/*****************************************************************************
** File:
**      Language.h
**
** Copyright (C) 2003-2004 Daniel Vik
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
******************************************************************************
*/
#ifndef LANGUAGE_FRENCH_H
#define LANGUAGE_FRENCH_H

#include "LanguageEnglish.h"

class LanguageFrench : public LanguageEnglish
{
public:
    LanguageFrench() {
        trainerName             = "Trainer";
        trainerCaption          = "blueMSX - Trainer";
        saveCheatCaption        = "Sauver Triches";
        loadCheatCaption        = "Charger Triches";
        pause                   = "Pause";
        run                     = "Lancer";
        truncated               = "tronqu�...";
        newCheat                = "Nouvelle Triche";
        address                 = "Adresse";
        oldValue                = "Ancienne Valeur";
        newValue                = "Nouvelle Valeur";
        change                  = "Changer";
        disable                 = "D�sactiver";
        enable                  = "Activer";
        description             = "Description";
        value                   = "Valeur";
        activeCheats            = "Triches Actives ";
        findCheats              = "Trouver des Triches ";
        removeAll               = "Tout retirer";
        remove                  = "Retirer";
        addCheat                = "Ajouter Triche";
        cheatFile               = "Fichier Triches:";
        ok                      = "OK";
        cancel                  = "Annuler";
        displayValueAs          = "Afficher la valeur ";
        decimal                 = "D�cimale";
        hexadecimal             = "Hexad�cimale";
        dataSize                = "Format des donn�es ";
        eightBit                = "8 bit";
        sixteenBit              = "16 bit";
        compareType             = "Mode de comparaison ";
        equal                   = "Egal";
        notEqual                = "Pas �gal";
        lessThan                = "Moins que";
        lessOrEqual             = "Moins ou �gal";
        greaterThan             = "Plus grand que";
        greaterOrEqual          = "Plus grand ou �gal";
        display                 = "Afficher ";
        compareNewValueWith     = "Comparer la nouvelle valeur avec ";
        specificValue           = "Valeur sp�cifique: ";
        snapshot                = "Snapshot";
        search                  = "Rechercher";
        undo                    = "Annuler";
    }
};

#endif //LANGUAGE_FRENCH_H
