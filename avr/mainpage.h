/** @file 
 * Projektmanualens huvudsida. 
 * @mainpage
 *
 * @section PlogenPlogen PlogenPlogen
 * 
 * Detta är kodreferensen för ett projekt i kursen TSEA27 Elektronikprojekt Y.
 *
 * Projektets mål var att bygga en robot som autonomt kan hitta genom en labyrint och hämta ett objekt.
 *
 * Roboten är uppdelad i tre moduler Styr, Sensor och Kommunikation. Koden är dock uppdelad i fyra delar, 
 * en för varje modul samt en gemensam del som innehåller kod för bussen.
 * 
 * @subsection Styrenheten Styrenhet
 * Styrenheten kontrollerar robotens motorer och logiken som styr dem. 
 * Den ansvarar för att hitta genom labyrinten och att hitta tillbaka ut.
 * Styrenheten kontrollerar också sensorenheten genom att bestämma vilket läge den ska arbeta i.
 *
 * Koden som kontrollerar styrenheten ligger i \ref styr
 *
 * @subsection Sensorenhet Sensorenhet
 * Sensorenheten hanterar robotens sensorer. Den gör också behandling av data den får från dem
 * innan den skickas till styrenheten. Dels skickas linjäriserad data som används av styrenheten 
 * för att reglerar roboten i labyrinten. Sensorenheten försöker också se hur labyrinten ser ut runt roboten
 * och meddelar detta till styrenheten som sedan tar beslut om vad som ska göras. 
 * Sensorenheten har också sensorer för linjeföjning och ett gyro för att kunna mäta rotation.
 *
 * Koden som kontrollerar sensorenheten ligger \ref sensor
 *
 * @subsection Kommunikationsenhet Kommunikationsenhet
 * Kommunikationsenheten sköter kommunikationen med dator som kan kopplas till roboten via blåtand.
 * Den vidarebefodrar data mellan bussen som kommunicerar mellan enheterna på roboten och blåtandsinterfacet.
 *
 * Koden som kontrollerar kommunikationsenheten ligger i \ref komm
 *
 * @subsection buss_protokoll I2C-buss och Protokoll
 * Koden som kotrollerar bussen och protokollet som används för att skicka data är gemensam för alla enheter.
 * Därför används samma kod i alla enheter. Denna gemensamma kod ligger i en egen modul och används i alla enheter.
 *
 * Denna kod ligger i modulen \ref buss
 *
 * @subsection Designfilosofi Designfilosofi
 * För att underlätta programmeringen av roboten har en del val gjorts om hur koden ska skrivas.
 * - Den kod som kan vara gemensam för alla moduler, framförallt busskod, är skriven så den kan användas 
 *   av alla enheter.
 * - Modulerna ska så mycket som möjligt vara oberoende av varandra. Kod ska så lite som möjligt bero på att någon annan
 *   enhet gör saker i en viss ordning eller på en viss tid. De beroenden som finns är att sensorenheten är i det 
 *   läge styrenheten tror att den är. Detta säkerställs genom att det alltid är styrenheten som bestämmer när
 *   sensorenheten byter läge.
 * - Inga tidsfördröjningar där någon enhet bara väntar på att något ska hända får finnas. Detta är framförallt 
 *   pga av att bussens buffertar skulle kunna bli fulla. Detta gör också att man aldrig slösar med processortid.
 *   Detta innebär också att alla enheter kan köra saker hela tiden, exempelvis finns inga tidsberoenden i reglerloopen.
 *   Så fort sensorvärden ändra kommer det visa sig i styrsignaler. Begränsningen är busskommunikationen som inte klarar 
 *   hur höga hastigheter som helst.
 * - Det ska finnas tydliga abstraktionsnivåer i koden. Hårdvara ska gömmas bakom funktioner och skiljas från logik.
 *   Exempelvis är bussen och dess buffertar helt gömda bakom två funktioner. Ovanpå det ligger kod som hanterar protokollet.
 *   Även alla styrmodulens lägen är en form av abstraktion som gömmer statusvariabler och den specifika hanteringen av 
 *   varje läge.
 *
 */
