# Logigramme nfc inventaire
240516.1453

## Variables
newRFID 
tagFromager 
procNewFromage
procInventaireFromage
procNotation
procNewTagCmd


## Logique
un TAG est lu !

if (is not in table tag cmd)
    tagUnknow = true
    if (procNewFromage) 
        tagUnknow = false
        procédure NewFromage
    if (procInventaire) 
        tagUnknow = false
        procédure Inventaire
    if (procNotation) 
        tagUnknow = false
        procédure Notation
    if (procNewTagCmd) 
        tagUnknow = false
        procédure NewTagCmd 
    if (tagUnknow)
        print("007, on a un problème tag inconnu !")
else
    if (tagFromager cmd) 
        tagFromager = newRFID 
    if (NewFromage cmd)
        clearAllProcedures
        procNewFromage = true 
    if (InventaireFromage cmd) 
        clearAllProcedures
        procInventaireFromage = true 
    if (Notation cmd) 
        clearAllProcedures
        procNotation = true 
    if (NewTagCmd cmd)
        clearAllProcedures
        Add next tag to table tag cmd




clearAllProcedures
procNewFromage = false
procInventaireFromage = false
procNotation = false
procNewTagCmd = false














.