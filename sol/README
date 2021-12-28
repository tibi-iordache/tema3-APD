Iordache Tiberiu-Mihai 336CA

							Tema 3 APD

- Citire date din fisiere: 
	Ficare coordonator citeste din fisierul atribuit acestuia si isi stocheaza
rangurile workerilor asignati acestuia.

- Stabilirea topologiei:
	- coordonator:
	In primul rand, fiecare coordonator isi va trimite rangul catre fiecare 
worker atribuit acestuia. Dupa care, el va trimite rangurile workerilor asociati
lui catre ceilalti coordonatori (intai urmatorului coordonator, dupa care
coordonatorului anterior acestuia; am considerat ca acestia sunt configurati
intr-un inel, astfel coordonatorul urmator corespunzator lui 2 este 0). Odata
ce fiecare coordonator primeste rangurile workerilor celorlalti coordonatori, 
el si construieste topologia(stocata ca un map unde cheile sunt rangurile 
coordonatorilor, iar valorile vectori cu rangurile workerilor) si o afiseaza.

	- worker:
	Fiecare worker primeste intai rangul coordonatorului acestuia(de la orice
sursa). Odata ce si-a stabilit coordonatorul, va primii de la acesta datele 
topologiei: rangul fiecarui coordonator si rangurile workerilor asociati acestuia.
Topologia se construieste similiar cu structura de date de la coordonatori,
odata ce este gata, o afiseaza.

- Realizarea calculelor: 
	- coordonator MASTER:
	Coorodonatorul cu rang 0 va incepe prin a genera vectorul. Vectorul va fi
impartit in mod egal la numarul total de workeri din intreaga topologie. Intai 
se vor trimite primele valori catre workerii coordonatorului MASTER, dupa care 
restul vectorului va fi distribuit catre ceilalti coordonatori. In final, acest
coordonator asteapta sa primeasca valorile dublate de la workerii sai, cat si de 
la ceilalti coordonatori pentru a asambla vectorul final si a-l afisa.

	- coordonator:
	Restul coordonatorilor vor astepta sa primeasca bucati din vector de la 
coordonatorul MASTER, dupa care le vor distribuii in mod egal workerilor si vor
astepta sa primeasca inapoi valorile dublate de catre workeri, pentru a le trimite
inapoi catre MASTER.

	- worker:
	Fiecare worker va astepta sa primeasca fragmenete din vector de la 
coordonatorul sau si va dubla fiecare element primit. Odata ce a primit tot, 
trimite inapoi datele dublate catre coordonator.

- Trimiterea datelor:
	In general majoritatea datelor trimise de tip vector sunt trimise dupa 
conventia urmatoare: trimit intai dimensiunea vectorului, dupa care vectorul
efectiv(dupa caz, acesta poate fi trimis element cu element sau de-a intregul).

- Logarea mesajelor:
	Pentru a afisa mesajele am folosit stringstream construind mesajele inainte
de afisare, astfel incat sa evit pe cat de mult posibil suprapunerea afisarilor
de la procese multiple.
