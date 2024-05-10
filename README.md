# NFC_inventaire
Système pour lire une puce NFC et donner des critères de qualité dans une DB API REST

zf240510.2158

**ATTENTION, documentation en cours de rédaction et le schéma n'est pas encore fait :-(  zf240510.1444**


## Buts
Système pour lire une puce NFC et donner des critères de qualité dans une DB API REST.


## Problématiques
Comme ce système va être utilisé dans une fromagerie pour inventorier des fromages dans une cave d'affinage, l'environnement est très agréssif, de l'eau salée partout et une humidité de 90%

Il faudra que le système soit totalement waterproof à la projection et que l'interface utilisateur soit avec des *boutons* capacitifs hyper simple à utiliser pour envoyer les caractéristiques des fromages dans la DB.


## Moyens
J'ai utilisé pour ce projet un esp32-c3 super mini

https://grabcad.com/library/esp32-c3-supermini-1

https://fr.aliexpress.com/item/1005006170575141.html

https://www.studiopieters.nl/esp32-c3-pinout/

https://forum.fritzing.org/t/need-esp32-c3-super-mini-board-model/20561


Un lecteur de puces NFC

https://www.aliexpress.com/item/1005006005040320.html

https://randomnerdtutorials.com/security-access-using-mfrc522-rfid-reader-with-arduino/

https://www.instructables.com/Using-Mifare-Ultralight-C-With-RC522-on-Arduino/


https://www.arduino.cc/reference/en/libraries/mfrc522/

https://github.com/miguelbalboa/rfid/tree/master/doc/fritzing


Un interrupteur analogique

https://www.aliexpress.com/item/1005003404607069.html


Des switches capacitifs

https://www.aliexpress.com/item/1005006087171183.html


## Sources
https://forum.fritzing.org/t/need-esp32-c3-super-mini-board-model/20561

https://www.digikey.fr/fr/resources/conversion-calculators/conversion-calculator-voltage-divider


