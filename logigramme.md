# Logigramme nfc inventaire
240520.1424

## Variables
newRFID 
tagFromager
tagNotation

procFromager
procAddFromage
procAddInventaire
procAddTagCmd
procNotation


## Logique
un TAG est lu !

if (is in table tag cmd)
    clearAllProcedures
    if (tagFromager cmd) 
        procFromager = true 
    if (tagAddFromage cmd) 
        procAddFromage = true 
    if (tagAddInventaire cmd) 
        procAddInventaire = true 
    if (tagAddTagCmd cmd) 
        procAddTagCmd = true 
    if (tagNotation cmd) 
        procNotation = true 
else
    tagUnknow = true
    if (procFromager) 
        tagUnknow = false
        procédure procFromager
    if (procAddFromage) 
        tagUnknow = false
        procédure procAddFromage
    if (procAddInventaire) 
        tagUnknow = false
        procédure procAddInventaire
    if (procAddTagCmd) 
        tagUnknow = false
        procédure procAddTagCmd
    if (procNotation) 
        tagUnknow = false
        procédure procNotation
    if (tagUnknow)
        print("007, on a un problème tag inconnu !")






clearAllProcedures
    procFromager = false
    procAddFromage = false
    procAddInventaire = false
    procAddTagCmd = false
    procNotation = false






.