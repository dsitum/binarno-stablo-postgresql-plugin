#include <postgres.h>
#include <fmgr.h>
#include <string.h>
#include <stdlib.h>
#include "stablo.h"
#include "parsiranje.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

#define popuniStablo(a, b, c) popuni_stablo(a, b, c, INDEKS_KORIJENA_STABLA)

Datum stablo_in(PG_FUNCTION_ARGS);
Datum stablo_out(PG_FUNCTION_ARGS);
Datum stablo_broj_cvorova(PG_FUNCTION_ARGS);
Datum stablo_broj_cvorova_podstabla(PG_FUNCTION_ARGS);
Datum stablo_postoji_cvor(PG_FUNCTION_ARGS);
Datum stablo_vrijednost(PG_FUNCTION_ARGS);
Datum stablo_promijeni_vrijednost(PG_FUNCTION_ARGS);
Datum stablo_dodaj_cvor(PG_FUNCTION_ARGS);
Datum stablo_orezi(PG_FUNCTION_ARGS);
Datum stablo_podstablo(PG_FUNCTION_ARGS);
Datum stablo_dodaj_podstablo(PG_FUNCTION_ARGS);
Datum stablo_preorder(PG_FUNCTION_ARGS);
Datum stablo_inorder(PG_FUNCTION_ARGS);
Datum stablo_postorder(PG_FUNCTION_ARGS);
Datum stablo_pretrazi(PG_FUNCTION_ARGS);
Datum stablo_eq(PG_FUNCTION_ARGS);
Datum stablo_neq(PG_FUNCTION_ARGS);
int pronadjiIndeksCvora(bstablo *stablo, char *putanja);
char *pronadjiPutanjuCvora(bstablo *stablo, int indeksCvora);
int brojCvorovaPodstabla(int indeksPocetnogCvora, bstablo *stablo);
void popuni_stablo(bstablo *podstablo, bstablo *originalnoStablo, int indeksCvora, int indeksCvoraPodstabla);
void preorder(bstablo *stablo, int pocetniIndeksCvora, char *izlaz);
void inorder(bstablo *stablo, int pocetniIndeksCvora, char *izlaz);
void postorder(bstablo *stablo, int pocetniIndeksCvora, char *izlaz);
void ispisiCvor(int idCvora, int vrijednost, int idLijevo, int idDesno, char *izlaz);

PG_FUNCTION_INFO_V1(stablo_in);
PG_FUNCTION_INFO_V1(stablo_out);
PG_FUNCTION_INFO_V1(stablo_broj_cvorova);
PG_FUNCTION_INFO_V1(stablo_broj_cvorova_podstabla);
PG_FUNCTION_INFO_V1(stablo_postoji_cvor);
PG_FUNCTION_INFO_V1(stablo_vrijednost);
PG_FUNCTION_INFO_V1(stablo_promijeni_vrijednost);
PG_FUNCTION_INFO_V1(stablo_dodaj_cvor);
PG_FUNCTION_INFO_V1(stablo_orezi);
PG_FUNCTION_INFO_V1(stablo_podstablo);
PG_FUNCTION_INFO_V1(stablo_dodaj_podstablo);
PG_FUNCTION_INFO_V1(stablo_preorder);
PG_FUNCTION_INFO_V1(stablo_inorder);
PG_FUNCTION_INFO_V1(stablo_postorder);
PG_FUNCTION_INFO_V1(stablo_pretrazi);
PG_FUNCTION_INFO_V1(stablo_eq);
PG_FUNCTION_INFO_V1(stablo_neq);



/*
Priprema ulazni niz znakova za pohranu u bazu podataka
 */
Datum stablo_in(PG_FUNCTION_ARGS) {
	char *unos = PG_GETARG_CSTRING(0);
	bstablo *rezultat;

	rezultat = pretvoriUStablo(unos);

	PG_RETURN_POINTER(rezultat);
}



/*
Pretvara stablo iz baze podataka u izlazni niz znakova
 */
Datum stablo_out(PG_FUNCTION_ARGS) {
	bstablo *stablo = (bstablo *) PG_GETARG_POINTER(0);
	char *izlaz = (char *) palloc(10000);

	// alociranoj varijabli izlaz za prvi znak postavljamo terminator znak
	izlaz[0] = '\0';
	preorder(stablo, INDEKS_KORIJENA_STABLA, izlaz);

	PG_RETURN_CSTRING(izlaz);
}




/*
Vraća ukupan broj čvorova u stablu
 */
Datum stablo_broj_cvorova(PG_FUNCTION_ARGS) {
	bstablo *stablo = (bstablo *) PG_GETARG_POINTER(0);
	int brojCvorova = 0, i;

	for (i = INDEKS_KORIJENA_STABLA; i < MAX_BROJ_CVOROVA; i++) {
		if (stablo[i].iskoristen) {
			brojCvorova++;
		}
	}

	PG_RETURN_INT32(brojCvorova);
}



/*
Vraća ukupan broj čvorova u stablu koji se nalaze ispod nekog čvora
Zbroj uključuje i polazni čvor!
 */
Datum stablo_broj_cvorova_podstabla(PG_FUNCTION_ARGS) {
	bstablo *stablo = (bstablo *) PG_GETARG_POINTER(0);
	char *putanja = (char *) PG_GETARG_CSTRING(1);
	int indeksCvora, brojCvorovaLijevo, brojCvorovaDesno;

	indeksCvora = pronadjiIndeksCvora(stablo, putanja);

	if (indeksCvora == NEPOSTOJECI_CVOR) {
		ereport(ERROR,
			(errcode(ERRCODE_RAISE_EXCEPTION),
				errmsg("Taj cvor jos nije alociran!\n"))
		);
	}

	brojCvorovaLijevo = brojCvorovaPodstabla(lijevoDijeteB(indeksCvora, stablo), stablo);
	brojCvorovaDesno = brojCvorovaPodstabla(desnoDijeteB(indeksCvora, stablo), stablo);

	return brojCvorovaLijevo + brojCvorovaDesno + 1;
}



/*
Vraća informaciju o tome postoji li odabrani čvor ili ne
*/
Datum stablo_postoji_cvor(PG_FUNCTION_ARGS) {
	bstablo *stablo = (bstablo *) PG_GETARG_POINTER(0);
	char *putanja = (char *) PG_GETARG_CSTRING(1);
	int indeksCvora;

	indeksCvora = pronadjiIndeksCvora(stablo, putanja);

	if (indeksCvora == NEPOSTOJECI_CVOR) {
		PG_RETURN_BOOL(FALSE);	
	} else {
		PG_RETURN_BOOL(TRUE);
	}
}



/*
Vraća vrijednost čvora na odabranoj putanji u stablu
 */
Datum stablo_vrijednost(PG_FUNCTION_ARGS) {
	bstablo *stablo = (bstablo *) PG_GETARG_POINTER(0);
	char *putanja = (char *) PG_GETARG_CSTRING(1);
	int indeksCvora, v = 0;

	indeksCvora = pronadjiIndeksCvora(stablo, putanja);

	if (indeksCvora == NEPOSTOJECI_CVOR) {
		ereport(ERROR,
			(errcode(ERRCODE_RAISE_EXCEPTION),
				errmsg("Taj cvor nije jos definiran!\n"))
		);
	} else {
		v = vrijednostB(indeksCvora, stablo);
	}

	PG_RETURN_INT32(v);
}



/*
Mijenja vrijednost čvora na odabranoj putanji u stablu
 */
Datum stablo_promijeni_vrijednost(PG_FUNCTION_ARGS) {
	bstablo *s = (bstablo *) PG_GETARG_POINTER(0);
	char *putanja = (char *) PG_GETARG_CSTRING(1);
	int v = (int) PG_GETARG_INT32(2);
	bstablo *stablo = kopirajStabloB(s);
	int indeksCvora;

	indeksCvora = pronadjiIndeksCvora(stablo, putanja);

	if (indeksCvora == NEPOSTOJECI_CVOR) {
		ereport(ERROR,
			(errcode(ERRCODE_RAISE_EXCEPTION),
				errmsg("Taj cvor nije jos definiran!\n"))
		);
	} else {
		promijeniVrijednostB(v, indeksCvora, stablo);
	}

	PG_RETURN_POINTER(stablo);
}



/*
Dodaje dijete čvoru na zadanoj putanji.
Putanja zapravo predstavlja putanju do djeteta, tako da vrijedi sljedeće:
->Putanja roditelja se promatra do predzadnjeg 'L' ili 'R'.
->Posljednji znak putanje će stvoriti lijevo dijete ako je 'L'
odnosno desno dijete ako je 'R'
 */
Datum stablo_dodaj_cvor(PG_FUNCTION_ARGS) {
	bstablo *s = (bstablo *) PG_GETARG_POINTER(0);
	char *putanjaDjeteta = (char *) PG_GETARG_CSTRING(1);
	int vrijednostDjeteta = (int) PG_GETARG_INT32(2);
	int indeksCvoraRoditelja;

	bstablo *stablo = kopirajStabloB(s);

	char *putanjaRoditelja = (char *) palloc(strlen(putanjaDjeteta));
	char pozicijaDjeteta;

	// u putanju roditelja kopiramo putanju djeteta bez posljednjeg znaka
	strncpy(putanjaRoditelja, putanjaDjeteta, strlen(putanjaDjeteta) - 1);
	// postavljamo null-character na kraj putanje roditelja
	putanjaRoditelja[strlen(putanjaDjeteta) - 1] = '\0';
	// u poziciju djeteta stavljamo vrijednost posljednjeg znaka putanje djeteta
	// pozicija djeteta je relativna s obzirom na putanju roditelja
	pozicijaDjeteta = putanjaDjeteta[strlen(putanjaDjeteta) - 1];

	// pronalazimo najprije indeks čvora roditelja
	indeksCvoraRoditelja = pronadjiIndeksCvora(stablo, putanjaRoditelja);

	pfree(putanjaRoditelja);

	// ako roditelj ne postoji, izbacujemo grešku
	if (indeksCvoraRoditelja == NEPOSTOJECI_CVOR) {
		ereport(ERROR,
			(errcode(ERRCODE_RAISE_EXCEPTION),
				errmsg("Cvor roditelja jos nije definiran!\n"))
		);
	}

	// ako roditelj postoji, provjeravamo je li zauzeto njegovo
	// lijevo ili desno dijete, ovisno o tome na koju stranu
	// želimo alocirati novo dijete
	if (pozicijaDjeteta == 'L') {

		if (lijevoDijeteB(indeksCvoraRoditelja, stablo) == NEPOSTOJECI_CVOR) {
			stvoriLijevoB(vrijednostDjeteta, indeksCvoraRoditelja, stablo);
		} else {
			ereport(ERROR,
				(errcode(ERRCODE_RAISE_EXCEPTION),
					errmsg("Lijevo dijete cvora roditelja je vec zauzeto!\n"))
			);
		}

	} else {

		if (desnoDijeteB(indeksCvoraRoditelja, stablo) == NEPOSTOJECI_CVOR) {
			stvoriDesnoB(vrijednostDjeteta, indeksCvoraRoditelja, stablo);
		} else {
			ereport(ERROR,
				(errcode(ERRCODE_RAISE_EXCEPTION),
					errmsg("Desno dijete cvora roditelja je vec zauzeto!\n"))
			);
		}
	}

	PG_RETURN_POINTER(stablo);
}




/*
Briše sve čvorove na zadanoj putanji (uključujući i čvor i njegovu djecu).
IZNIMKA! Korijen stabla nije moguće obrisati. U slučaju pokušaja brisanja korijena,
obrisat će se njegovo lijevo i desno podstablo
 */
Datum stablo_orezi(PG_FUNCTION_ARGS) {
	bstablo *s = (bstablo *) PG_GETARG_POINTER(0);
	char *putanja = (char *) PG_GETARG_CSTRING(1);

	bstablo *stablo = kopirajStabloB(s);

	int indeksCvora = pronadjiIndeksCvora(stablo, putanja);

	if (indeksCvora == NEPOSTOJECI_CVOR) {
		ereport(ERROR,
			(errcode(ERRCODE_RAISE_EXCEPTION),
				errmsg("Stablo na toj putanji ne postoji!\n"))
		);
	}

	// ako je čvor koji se želi obrisati korijen stabla, tada će se obrisati lijeva i desna strana korijena
	// a u suprotnom će se obrisati navedeni čvor i sva njegova djeca
	if (indeksCvora == INDEKS_KORIJENA_STABLA) {
		obrisiCvorB(lijevoDijeteB(indeksCvora, stablo), stablo);
		obrisiCvorB(desnoDijeteB(indeksCvora, stablo), stablo);
	} else {
		obrisiCvorB(indeksCvora, stablo);
	}

	PG_RETURN_POINTER(stablo);
}



/*
Vraća podstablo na zadanoj putanji.
 */
Datum stablo_podstablo(PG_FUNCTION_ARGS) {
	bstablo *stablo = (bstablo *) PG_GETARG_POINTER(0);
	char *putanja = (char *) PG_GETARG_CSTRING(1);

	// najprije pronalazimo indeks čvora čije podstablo želimo pronaći
	int indeksCvora = pronadjiIndeksCvora(stablo, putanja);
	bstablo *podstablo;

	if (indeksCvora == NEPOSTOJECI_CVOR) {
		ereport(ERROR,
			(errcode(ERRCODE_RAISE_EXCEPTION),
				errmsg("Stablo na toj putanji ne postoji!\n"))
		);
	}

	// potom najprije alociramo podstablo i inicijaliziramo njegov korijen na
	// vrijednosti čvora u originalnom stablu
	podstablo = inicijalizirajB(vrijednostB(indeksCvora, stablo));

	// potom koristimo pomoćnu rekurzivnu funkciju koja će nam pomoći popuniti alocirano podstablo sa 
	// čvorovima originalnog stabla
	popuniStablo(podstablo, stablo, indeksCvora);

	PG_RETURN_POINTER(podstablo);
}



/*
Dodaje podstablo na čvor nekog stabla. Taj čvor je označen putanjom, na isti način
kao i kod funkcije stablo_dodaj_cvor. Jedina razlika je ta što ova funkcija 
ne dodaje novi čvor, nego čitavo novo podstablo
 */
Datum stablo_dodaj_podstablo(PG_FUNCTION_ARGS) {
	bstablo *s = (bstablo *) PG_GETARG_POINTER(0);
	char *putanjaPodstabla = (char *) PG_GETARG_CSTRING(1);
	bstablo *podstablo = pretvoriUStablo( (char *) PG_GETARG_POINTER(2) );
	bstablo *stablo = kopirajStabloB(s);
	int indeksCvoraRoditelja;

	char *putanjaRoditelja = (char *) palloc(strlen(putanjaPodstabla));
	char pozicijaPodstabla;  // lijevo ili desno u odnosu na čvor roditelja

	// u putanju čvora roditelja kopiramo putanju djeteta bez posljednjeg znaka
	strncpy(putanjaRoditelja, putanjaPodstabla, strlen(putanjaPodstabla) - 1);
	// postavljamo null-character na kraj putanje čvora roditelja
	putanjaRoditelja[strlen(putanjaPodstabla) - 1] = '\0';
	// u poziciju djeteta stavljamo vrijednost posljednjeg znaka putanje djeteta
	// pozicija djeteta je relativna s obzirom na putanju do čvora roditelja
	pozicijaPodstabla = putanjaPodstabla[strlen(putanjaPodstabla) - 1];

	// pronalazimo najprije indeks čvora roditelja
	indeksCvoraRoditelja = pronadjiIndeksCvora(stablo, putanjaRoditelja);

	pfree(putanjaRoditelja);


	// ako roditelj ne postoji, izbacujemo grešku
	if (indeksCvoraRoditelja == NEPOSTOJECI_CVOR) {
		ereport(ERROR,
			(errcode(ERRCODE_RAISE_EXCEPTION),
				errmsg("Cvor roditelja jos nije definiran!\n"))
		);
	}

	// ako roditelj postoji, provjeravamo je li zauzeto njegovo
	// lijevo ili desno dijete, ovisno o tome na koju stranu
	// želimo dodati podstablo
	if (pozicijaPodstabla == 'L') {

		if (lijevoDijeteB(indeksCvoraRoditelja, stablo) == NEPOSTOJECI_CVOR) {
			int indeksLijevogDjeteta = indeksCvoraRoditelja*2;
			popuni_stablo(stablo, podstablo, INDEKS_KORIJENA_STABLA, indeksLijevogDjeteta);
		} else {
			ereport(ERROR,
				(errcode(ERRCODE_RAISE_EXCEPTION),
					errmsg("Odabrana putanja vec ima lijevo dijete!\n"))
			);
		}

	} else {

		if (desnoDijeteB(indeksCvoraRoditelja, stablo) == NEPOSTOJECI_CVOR) {
			int indeksDesnogDjeteta = indeksCvoraRoditelja*2+1;
			popuni_stablo(stablo, podstablo, INDEKS_KORIJENA_STABLA, indeksDesnogDjeteta);
		} else {
			ereport(ERROR,
				(errcode(ERRCODE_RAISE_EXCEPTION),
					errmsg("Odabrana putanja vec ima desno dijete!\n"))
			);
		}
	}

	PG_RETURN_POINTER(stablo);
}



/*
Preorder ispis stabla
 */
Datum stablo_preorder(PG_FUNCTION_ARGS) {
	bstablo *stablo = (bstablo *) PG_GETARG_POINTER(0);
	char *izlaz = (char *) palloc(10000);

	// alociranoj varijabli izlaz za prvi znak postavljamo terminator znak
	izlaz[0] = '\0';
	preorder(stablo, INDEKS_KORIJENA_STABLA, izlaz);

	PG_RETURN_CSTRING(izlaz);
}



/*
Inorder ispis stabla
 */
Datum stablo_inorder(PG_FUNCTION_ARGS) {
	bstablo *stablo = (bstablo *) PG_GETARG_POINTER(0);
	char *izlaz = (char *) palloc(10000);

	// alociranoj varijabli izlaz za prvi znak postavljamo terminator znak
	izlaz[0] = '\0';
	inorder(stablo, INDEKS_KORIJENA_STABLA, izlaz);

	PG_RETURN_CSTRING(izlaz);
}



/*
Postorder ispis stabla
 */
Datum stablo_postorder(PG_FUNCTION_ARGS) {
	bstablo *stablo = (bstablo *) PG_GETARG_POINTER(0);
	char *izlaz = (char *) palloc(10000);

	// alociranoj varijabli izlaz za prvi znak postavljamo terminator znak
	izlaz[0] = '\0';
	postorder(stablo, INDEKS_KORIJENA_STABLA, izlaz);

	PG_RETURN_CSTRING(izlaz);
}




/*
Pretražuje sve čvorove stabla i uspoređuje ih s odabranom vrijednošću.
Vraća jednu ili više putanja do onih čvorova koji se poklapaju s
odabranom vrijednošću
 */
Datum stablo_pretrazi(PG_FUNCTION_ARGS) {
	bstablo *stablo = (bstablo *) PG_GETARG_POINTER(0);
	int vrijednost = (int) PG_GETARG_INT32(1);

	int i;
	// ovdje ćemo bilježiti sve putanje do čvorova
	char *putanje = (char *) palloc(1000);
	// ovdje ćemo bilježiti putanju do pojedinog čvora
	char *putanja;

	// null-terminiramo varijablu putanje
	putanje[0] = '\0';


	// tražimo čvorove koji se poklapaju sa zadanom vrijednošću
	for (i = INDEKS_KORIJENA_STABLA; i < MAX_BROJ_CVOROVA; i++) {
		// kada smo našli čvor koji se podudara
		if (stablo[i].iskoristen == 1 && stablo[i].v == vrijednost) {
			// tražimo njegovu putanju
			putanja = pronadjiPutanjuCvora(stablo, i);

			// ako je putanja prazna, to znači da se radi o korijenu
			if (strlen(putanja) == 0) {
				strcpy(putanja, "K");
			}

			// dodajemo dobivenu putanju u listu putanja
			// u ovisnosti o tome je li lista inicijalizirana ili ne
			if (strlen(putanje) == 0) {
				strcpy(putanje, putanja);
			} else {
				strcat(putanje, ", ");
				strcat(putanje, putanja);
			}

			pfree(putanja);
		}
	}

	// na kraju, ako nije pronađena niti jedna vrijednost,
	// u rezultat umjesto putanje upisujemo poruku o tome
	// da niti jedan čvor s tom vrijednošću nije pronađen
	if (strlen(putanje) == 0) {
		strcpy(putanje, "Ne postoji cvor s tom vrijednoscu!");
	}

	PG_RETURN_CSTRING(putanje);
}



/*
Vraća informaciju o tome jesu li dva binarna stabla jednaka
*/
Datum stablo_eq(PG_FUNCTION_ARGS) {
	bstablo *stablo1 = (bstablo *) PG_GETARG_POINTER(0);
	bstablo *stablo2 = (bstablo *) PG_GETARG_POINTER(1);

	if (jednakiB(stablo1, stablo2))
		PG_RETURN_BOOL(TRUE);
	else
		PG_RETURN_BOOL(FALSE);
}



/*
Vraća informaciju o tome jesu li dva binarna stabla različita
*/
Datum stablo_neq(PG_FUNCTION_ARGS) {
	bstablo *stablo1 = (bstablo *) PG_GETARG_POINTER(0);
	bstablo *stablo2 = (bstablo *) PG_GETARG_POINTER(1);

	if (! jednakiB(stablo1, stablo2))
		PG_RETURN_BOOL(TRUE);
	else
		PG_RETURN_BOOL(FALSE);
}



///////////////////////////////////////  POMOĆNE FUNKCIJE  ///////////////////////////////////////////



/**
 * Pronalazi indeks čvora u stablu prema danom predlošku
 * @param  stablo    binarno stablo
 * @param  putanja Niz znakova koji se sastoji od 'L' i 'D', a koji govori
 * na koji se način treba kretati do čvora kako bi se došlo do traženog čvora
 * @return           Indeks u polju binarnog stabla od proanađenog čvora
 */
int pronadjiIndeksCvora(bstablo *stablo, char *putanja) {
	int indeksCvora = INDEKS_KORIJENA_STABLA;
	int i;

	for (i = 0; i < strlen(putanja); i++) {
		if (putanja[i] == 'L') {
			indeksCvora = lijevoDijeteB(indeksCvora, stablo);
		} else {
			indeksCvora = desnoDijeteB(indeksCvora, stablo);
		}
	}

	return indeksCvora;
}




/**
 * Pronalazi putanju do čvora sa zadanim indeksom čvora u stablu (stablo je polje)
 * @param stablo      binarno stablo
 * @param indeksCvora indeks čvora u stablu kojeg je potrebno pronaći
 * @return 			  String tipa 'LDDLD' koji označava putanju do traženog čvora
 */
char *pronadjiPutanjuCvora(bstablo *stablo, int indeksCvora) {
	// u implementaciji stabla uz pomoć polja možemo jednostavno pronaći putanju čvora,
	// tako da indeks čvora konstantno dijelimo s 2, dok ne dođemo do korijena stabla
	// ako je tako dobivana vrijednost indeksa negativna, radi se o desnom čvoru.
	// U suprotnom se radi o lijevom čvoru (jer je lijevi čvor x*2, a desni x*2+1)
	
	int i, duljinaPutanje = 0;

	// predstavlja putanju do čvora
	char *putanja = (char *) palloc(10);

	// tražimo putanju opisanom metodom
	// ovom metodom dobivamo obrnutu putanju (nastavak dalje)
	while (indeksCvora != INDEKS_KORIJENA_STABLA) {
		if (indeksCvora % 2 == 0) {
			putanja[duljinaPutanje] = 'L';
		} else {
			putanja[duljinaPutanje] = 'D';
		}
		duljinaPutanje++;

		indeksCvora /= 2;
	}

	// na kraju null-terminiramo putanju
	putanja[duljinaPutanje] = '\0';



	// budući da smo se kretali od traženog čvora, do korijena,
	// potrebno je napraviti inverz putanje
	// (kako bi dobili putanju od korijena do traženog čvora)
	// stoga radimo in-place inverz znakovnog niza putanja
	for (i = 0; i < duljinaPutanje / 2; i++) {
		char tmp;
		tmp = putanja[i];
		putanja[i] = putanja[duljinaPutanje - i - 1];
		putanja[duljinaPutanje - i - 1] = tmp;
	}

	return putanja;
}



/**
 * Rekurzivna pomoćna funckija koja broji čvorove nekog podstabla
 * @param  indeksPocetnogCvora Označava indeks čvora od kojeg počinje brojanje
 * @param  stablo              binarno stablo
 * @return                     Broj čvorova nekog podstabla
 */
int brojCvorovaPodstabla(int indeksPocetnogCvora, bstablo *stablo) {
	int sumaCvorova = 0, indeksLijevogDjeteta, indeksDesnogDjeteta;

	if (indeksPocetnogCvora == NEPOSTOJECI_CVOR) {
		return 0;
	}

	indeksLijevogDjeteta = lijevoDijeteB(indeksPocetnogCvora, stablo);
	indeksDesnogDjeteta = desnoDijeteB(indeksPocetnogCvora, stablo);

	if (indeksLijevogDjeteta == NEPOSTOJECI_CVOR && indeksDesnogDjeteta == NEPOSTOJECI_CVOR) {
		return 1;
	} else {
		sumaCvorova += brojCvorovaPodstabla(indeksLijevogDjeteta, stablo);
		sumaCvorova += brojCvorovaPodstabla(indeksDesnogDjeteta, stablo);
		return sumaCvorova + 1;
	}
}



/**
 * Popunjava podstablo s vrijednostima originalnog stabla,
 * počevši od indeksa čvora originalnog stabla
 * @param podstablo   			Podstablo koje treba popuniti
 * @param originalnoStablo  	Stablo iz kojeg se čitaju vrijedndosti
 * @param indeksCvora 			Indeks originalnog stabla od kojeg se počinju
 * popunjavati vrijednosti u podstablu
 * @param indeksCvoraPodstabla	Indeks čvora podstabla kojeg popunjavamo.
 * Ova vrijednost je automatski inicijalizirana na 1, zbog čega se ne treba proslijeđivati funkciji,
 * ali je potrebna, jer se mijenja svakoj sljedećoj iteraciji rekurzije
 */
void popuni_stablo(bstablo *podstablo, bstablo *originalnoStablo, int indeksCvora, int indeksCvoraPodstabla) {
	int indeksLijevogDjeteta, indeksDesnogDjeteta;

	if (indeksCvora == NEPOSTOJECI_CVOR || indeksCvoraPodstabla == NEPOSTOJECI_CVOR) {
		return;
	}

	podstablo[indeksCvoraPodstabla].v = originalnoStablo[indeksCvora].v;
	podstablo[indeksCvoraPodstabla].iskoristen = 1;

	indeksLijevogDjeteta = lijevoDijeteB(indeksCvora, originalnoStablo);
	indeksDesnogDjeteta = desnoDijeteB(indeksCvora, originalnoStablo);

	popuni_stablo(podstablo, originalnoStablo, indeksLijevogDjeteta, indeksCvoraPodstabla*2);
	popuni_stablo(podstablo, originalnoStablo, indeksDesnogDjeteta, indeksCvoraPodstabla*2+1);
}




/**
 * Rekurzivno ispisuje stablo načinom preorder u izlaznu varijablu
 * @param stablo             binarno stablo
 * @param pocetniIndeksCvora indeks čvora od kojeg kreće kretanje kroz stablo
 * @param izlaz              varijabla u koju se zapisuju rezultati ispisa
 */
void preorder(bstablo *stablo, int pocetniIndeksCvora, char *izlaz) {
	int indeksLijevogDjeteta, indeksDesnogDjeteta;
	int idLijevo, idDesno;

	if (pocetniIndeksCvora == NEPOSTOJECI_CVOR) return;

	indeksLijevogDjeteta = lijevoDijeteB(pocetniIndeksCvora, stablo);
	indeksDesnogDjeteta = desnoDijeteB(pocetniIndeksCvora, stablo);

	idLijevo = (indeksLijevogDjeteta == NEPOSTOJECI_CVOR) ? -1 : indeksLijevogDjeteta;
	idDesno = (indeksDesnogDjeteta == NEPOSTOJECI_CVOR) ? -1 : indeksDesnogDjeteta;

	// čvor se ispisuje u varijablu izlaz
	ispisiCvor(pocetniIndeksCvora, vrijednostB(pocetniIndeksCvora, stablo), idLijevo, idDesno, izlaz);

	// granamo se lijevo i desno
	preorder(stablo, indeksLijevogDjeteta, izlaz);
	preorder(stablo, indeksDesnogDjeteta, izlaz);
}





/**
 * Rekurzivno ispisuje stablo načinom inorder u izlaznu varijablu
 * @param stablo             binarno stablo
 * @param pocetniIndeksCvora indeks čvora od kojeg kreće kretanje kroz stablo
 * @param izlaz              varijabla u koju se zapisuju rezultati ispisa
 */
void inorder(bstablo *stablo, int pocetniIndeksCvora, char *izlaz) {
	int indeksLijevogDjeteta, indeksDesnogDjeteta;
	int idLijevo, idDesno;

	if (pocetniIndeksCvora == NEPOSTOJECI_CVOR) return;

	indeksLijevogDjeteta = lijevoDijeteB(pocetniIndeksCvora, stablo);
	indeksDesnogDjeteta = desnoDijeteB(pocetniIndeksCvora, stablo);

	idLijevo = (indeksLijevogDjeteta == NEPOSTOJECI_CVOR) ? -1 : indeksLijevogDjeteta;
	idDesno = (indeksDesnogDjeteta == NEPOSTOJECI_CVOR) ? -1 : indeksDesnogDjeteta;

	inorder(stablo, indeksLijevogDjeteta, izlaz);
	ispisiCvor(pocetniIndeksCvora, vrijednostB(pocetniIndeksCvora, stablo), idLijevo, idDesno, izlaz);
	inorder(stablo, indeksDesnogDjeteta, izlaz);
}




/**
 * Rekurzivno ispisuje stablo načinom inorder u izlaznu varijablu
 * @param stablo             binarno stablo
 * @param pocetniIndeksCvora indeks čvora od kojeg kreće kretanje kroz stablo
 * @param izlaz              varijabla u koju se zapisuju rezultati ispisa
 */
void postorder(bstablo *stablo, int pocetniIndeksCvora, char *izlaz) {
	int indeksLijevogDjeteta, indeksDesnogDjeteta;
	int idLijevo, idDesno;

	if (pocetniIndeksCvora == NEPOSTOJECI_CVOR) return;

	indeksLijevogDjeteta = lijevoDijeteB(pocetniIndeksCvora, stablo);
	indeksDesnogDjeteta = desnoDijeteB(pocetniIndeksCvora, stablo);

	idLijevo = (indeksLijevogDjeteta == NEPOSTOJECI_CVOR) ? -1 : indeksLijevogDjeteta;
	idDesno = (indeksDesnogDjeteta == NEPOSTOJECI_CVOR) ? -1 : indeksDesnogDjeteta;

	postorder(stablo, indeksLijevogDjeteta, izlaz);
	postorder(stablo, indeksDesnogDjeteta, izlaz);
	ispisiCvor(pocetniIndeksCvora, vrijednostB(pocetniIndeksCvora, stablo), idLijevo, idDesno, izlaz);
}






/**
 * Ispisuje čvor na kraj varijable izlaz
 * @param idCvora     ID čvora roditelja
 * @param vrijednost  vrijednost čvora roditelja
 * @param idLijevo 	  ID lijevog djeteta čvora
 * @param idDesno     ID desnog djeteta čvora
 * @param izlaz 	  Izlazni string čvorova
 */
void ispisiCvor(int idCvora, int vrijednost, int idLijevo, int idDesno, char *izlaz) {
	char *cvor = (char *) palloc(50);

	// stvaramo čvor od primljenih podataka
	snprintf(cvor, 50, "(%d,%d,%d,%d)", idCvora, vrijednost, idLijevo, idDesno);

	// i taj tekstualni čvor dodajemo na listu čvorova
	// u ovisnosti o tome je li lista čvorova prazna ili ne
	if (strlen(izlaz) == 0) {
		strcpy(izlaz, cvor);
	} else {
		strcat(izlaz, ";");
		strcat(izlaz, cvor);
	}

	pfree(cvor);
}