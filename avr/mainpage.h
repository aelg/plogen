/** @file 
 * Projektmanualens huvudsida. 
 * @mainpage
 *
 * @section PlogenPlogen PlogenPlogen
 * 
 * Detta �r kodreferensen f�r ett projekt i kursen TSEA27 Elektronikprojekt Y.
 *
 * Projektets m�l var att bygga en robot som autonomt kan hitta genom en labyrint och h�mta ett objekt.
 *
 * Roboten �r uppdelad i tre moduler Styr, Sensor och Kommunikation. Koden �r dock uppdelad i fyra delar, 
 * en f�r varje modul samt en gemensam del som inneh�ller kod f�r bussen.
 * 
 * @subsection Styrenheten Styrenhet
 * Styrenheten kontrollerar robotens motorer och logiken som styr dem. 
 * Den ansvarar f�r att hitta genom labyrinten och att hitta tillbaka ut.
 * Styrenheten kontrollerar ocks� sensorenheten genom att best�mma vilket l�ge den ska arbeta i.
 *
 * Koden som kontrollerar styrenheten ligger i \ref styr
 *
 * @subsection Sensorenhet Sensorenhet
 * Sensorenheten hanterar robotens sensorer. Den g�r ocks� behandling av data den f�r fr�n dem
 * innan den skickas till styrenheten. Dels skickas linj�riserad data som anv�nds av styrenheten 
 * f�r att reglerar roboten i labyrinten. Sensorenheten f�rs�ker ocks� se hur labyrinten ser ut runt roboten
 * och meddelar detta till styrenheten som sedan tar beslut om vad som ska g�ras. 
 * Sensorenheten har ocks� sensorer f�r linjef�jning och ett gyro f�r att kunna m�ta rotation.
 *
 * Koden som kontrollerar sensorenheten ligger \ref sensor
 *
 * @subsection Kommunikationsenhet Kommunikationsenhet
 * Kommunikationsenheten sk�ter kommunikationen med dator som kan kopplas till roboten via bl�tand.
 * Den vidarebefodrar data mellan bussen som kommunicerar mellan enheterna p� roboten och bl�tandsinterfacet.
 *
 * Koden som kontrollerar kommunikationsenheten ligger i \ref komm
 *
 * @subsection buss_protokoll I2C-buss och Protokoll
 * Koden som kotrollerar bussen och protokollet som anv�nds f�r att skicka data �r gemensam f�r alla enheter.
 * D�rf�r anv�nds samma kod i alla enheter. Denna gemensamma kod ligger i en egen modul och anv�nds i alla enheter.
 *
 * Denna kod ligger i modulen \ref buss
 *
 * @subsection Designfilosofi Designfilosofi
 * F�r att underl�tta programmeringen av roboten har en del val gjorts om hur koden ska skrivas.
 * - Den kod som kan vara gemensam f�r alla moduler, framf�rallt busskod, �r skriven s� den kan anv�ndas 
 *   av alla enheter.
 * - Modulerna ska s� mycket som m�jligt vara oberoende av varandra. Kod ska s� lite som m�jligt bero p� att n�gon annan
 *   enhet g�r saker i en viss ordning eller p� en viss tid. De beroenden som finns �r att sensorenheten �r i det 
 *   l�ge styrenheten tror att den �r. Detta s�kerst�lls genom att det alltid �r styrenheten som best�mmer n�r
 *   sensorenheten byter l�ge.
 * - Inga tidsf�rdr�jningar d�r n�gon enhet bara v�ntar p� att n�got ska h�nda f�r finnas. Detta �r framf�rallt 
 *   pga av att bussens buffertar skulle kunna bli fulla. Detta g�r ocks� att man aldrig sl�sar med processortid.
 *   Detta inneb�r ocks� att alla enheter kan k�ra saker hela tiden, exempelvis finns inga tidsberoenden i reglerloopen.
 *   S� fort sensorv�rden �ndra kommer det visa sig i styrsignaler. Begr�nsningen �r busskommunikationen som inte klarar 
 *   hur h�ga hastigheter som helst.
 * - Det ska finnas tydliga abstraktionsniv�er i koden. H�rdvara ska g�mmas bakom funktioner och skiljas fr�n logik.
 *   Exempelvis �r bussen och dess buffertar helt g�mda bakom tv� funktioner. Ovanp� det ligger kod som hanterar protokollet.
 *   �ven alla styrmodulens l�gen �r en form av abstraktion som g�mmer statusvariabler och den specifika hanteringen av 
 *   varje l�ge.
 *
 */
