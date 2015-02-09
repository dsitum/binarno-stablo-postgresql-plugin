/**
 * Ova datoteka služi da pomogne pri parsiranju stabla u obliku teksta i njegovoj pretvorbi u objekt stabla
 * Dakle, tekstualna reprezentacija stabla tipa "(1,153,2,N),(2,422,N,N)" će biti pretvorena u memorijsku reprezentaciju stabla
 */

#define CVOR_ISKORISTEN -1

/**
 * Ova struktura predstavlja privremeni čvor stabla. Svaki čvor stabla ima svoj privremeni čvor. 
 * Privremeni čvorovi se stvaraju kako bi olakšali pretvorbu iz tekstualne reprezentacije stabla u stvarno stablu.
 * Tekstualno stablo (korisnički unos) se, dakle, najprije pretvara u ovakve oblike privremenih čvorova, 
 * prije nego se pretvori u stvarno stablo
 * 
 * id: id čvora privremenog stabla
 * v: vrijednost privremenog čvora stabla
 * lijevo: id lijevog dijeteta 
 * desno: id desnog djeteta
 */
typedef struct tmp_cvor {
  int id, v, lijevo, desno;
} tmp_cvor;


bstablo *pretvoriUStablo(char *stablo_tekst);
int brojCvorovaUStablu(char *stablo);
tmp_cvor* popuniPrivremeneCvorove(char *stablo, int brojCvorova);
bstablo *pronadjiKorijenStabla(tmp_cvor *privremeniCvorovi, int brojCvorova, int *idKorijenaStabla);
void popuniStablo(bstablo *stablo, int indeksPocetnogCvora, int idPocetnogCvora, tmp_cvor *privremeniCvorovi, int brojCvorova);
void provjeriNeiskoristeneCvorove(tmp_cvor *privremeniCvorovi, int brojCvorova);


/**
 * Ova funkcija pretvara tekstualnu reprezentaciju stabla u binarno stablo
 * @param  stablo_tekst tekstualna reprezentacija stabla
 * @return              binarno stablo u memoriji
 */
bstablo *pretvoriUStablo(char *stablo_tekst) {
	int idKorijenaStabla = 0;

	int brojCvorova = brojCvorovaUStablu(stablo_tekst);
	tmp_cvor *privremeniCvorovi = popuniPrivremeneCvorove(stablo_tekst, brojCvorova);
	bstablo *stablo = pronadjiKorijenStabla(privremeniCvorovi, brojCvorova, &idKorijenaStabla);  // inicijalizira stablo
	popuniStablo(stablo, INDEKS_KORIJENA_STABLA, idKorijenaStabla, privremeniCvorovi, brojCvorova);
	provjeriNeiskoristeneCvorove(privremeniCvorovi, brojCvorova);

	pfree(privremeniCvorovi);
	return stablo;
}



/**
 * Vraća broj čvorova u tekstualnoj reprezentaciji stabla
 * @param  stablo Tekstualna reprezentacija stabla
 * @return        Broj čvorova
 */
int brojCvorovaUStablu(char *stablo) {
	int brojCvorova = 1, i;
	for(i = 0; i < strlen(stablo); i++) {  // sve dok ne dođemo do null znaka
		if (stablo[i] == ';') {
			brojCvorova++;
		}
	}
	return brojCvorova;
}



/**
 * Stvara privremene čvorove iz tekstualne reprezentacije stabla. 
 * Takvi privremeni čvorovi služe kako bi olakšali stvaranje stvarnog stabla
 * @param  stablo      Tekstualna reprezentacija stabla
 * @param  brojCvorova Broj čvorova u tekstualnoj reprezentaciji stabla
 * @return             Polje privremenih čvorova
 */
tmp_cvor* popuniPrivremeneCvorove(char *stablo, int brojCvorova) {
	int i;
	char *cvor;

	// najprije stvaramo polje privremenih čvorova
	tmp_cvor *privremeniCvorovi = (tmp_cvor *) palloc(brojCvorova * sizeof(struct tmp_cvor));

	// potom to polje popunjavamo s podacima iz tekstualne reprezentacije stabla
	cvor = strtok(stablo, ";");
	for (i = 0; i < brojCvorova; i++) {
		int id, v, l, d;
		if (sscanf(cvor, "(%d, %d, %d, %d)", &id, &v, &l, &d) != 4) {
			ereport(ERROR,
				(errcode(ERRCODE_RAISE_EXCEPTION),
					errmsg("Pogresna sintaksa cvora \"%s\"",cvor))
			);
		}


		// popunjavanje privremenog čvora (uz javljanje pogreški ukoliko dođe do njih)
		if (id >= 0) {
			privremeniCvorovi[i].id = id;
		} else {
			ereport(ERROR,
				(errcode(ERRCODE_RAISE_EXCEPTION),
					errmsg("ID cvora \"%s\" mora biti nenegativan broj\n", cvor))
			);
		}

		privremeniCvorovi[i].v = v;

		if (l >= -1) {
			privremeniCvorovi[i].lijevo = l;
		} else {
			ereport(ERROR,
				(errcode(ERRCODE_RAISE_EXCEPTION),
					errmsg("Lijevo dijete cvora \"%s\" mora biti nenegativan broj ili -1\n", cvor))
			);
		}

		if (d >= -1) {
			privremeniCvorovi[i].desno = d;
		} else {
			ereport(ERROR,
				(errcode(ERRCODE_RAISE_EXCEPTION),
					errmsg("Desno dijete cvora \"%s\" mora biti nenegativan broj ili -1\n", cvor))
			);
		}


		cvor = strtok(NULL, ";");
	}

	return privremeniCvorovi;
}


/**
 * Iz polja struktura privremenih čvorova pronalazi korijen stabla. Korijen stabla je onaj čvor koji nema roditelja, odnosno
 * kojega niti jedan drugi čvor nema za lijevo (ili desno) dijete
 * @param  privremeniCvorovi polje struktura privremenih čvorova
 * @param  brojCvorova       broj privremenih čvorova u strukturi privremenih čvorova (broj čvorova koje je korisnik unio)
 * @param  idKorijenaStabla  u ovu varijablu će se upisati ID pronađenog korijena stabla
 * @return                   memorijska reprezentacija binarnog stabla
 */
bstablo *pronadjiKorijenStabla(tmp_cvor *privremeniCvorovi, int brojCvorova, int *idKorijenaStabla) {
	int i,j, korijenPronadjen = 1, vrijednostKorijena;
	bstablo *stablo = NULL;

	for (i = 0; i < brojCvorova; i++) {
		for (j = 0; j < brojCvorova; j++) {
			if (privremeniCvorovi[j].lijevo == privremeniCvorovi[i].id || privremeniCvorovi[j].desno == privremeniCvorovi[i].id) {
				korijenPronadjen = 0;
				break;
			}
		}

		// ako je korijen stabla pronađen, stvaramo stvarno stablo i vraćamo ga
		// također zapisujemo vrijednost u varijablu idKorijenaStabla
		if (korijenPronadjen == 1) {
			vrijednostKorijena = privremeniCvorovi[i].v;
			stablo = inicijalizirajB(vrijednostKorijena);
			*idKorijenaStabla = privremeniCvorovi[i].id;
			return stablo;
		}

		// u drugom slučaju, postavljamo da je korijen pronađen i promatramo hoće li se u sljedećoj iteraciji
		// ugniježđene for petlje to promijeniti
		korijenPronadjen = 1;
	}

	free(privremeniCvorovi);
	// ako korijen stabla nije pronađen
	ereport(ERROR,
		(errcode(ERRCODE_RAISE_EXCEPTION),
			errmsg("Uneseni oblik stabla nema korijena!"))
	);

	return NULL;  // da se izbjegne warning pri kompiliranju
}



/**
 * Popunjava stablo s ostalim čvorovima i postupno dealocira privremene čvorove
 * @param  stablo              Pokazivač na binarno stablo
 * @param  indeksPocetnogCvora Indeks čvora od kojeg kreće popunjavanje stabla
 * @param  idPocetnogCvora     ID čvora od kojeg kreće popunjavanje stabla
 * @param  privremeniCvorovi   Polje privremenih čvorova iz kojeg će se dohvaćati čvorovi
 * @param  brojCvorova         Broj čvorova u polju privremenih čvorova
 */
void popuniStablo(bstablo *stablo, int indeksPocetnogCvora, int idPocetnogCvora, tmp_cvor *privremeniCvorovi, int brojCvorova) {
	int i;
	int idLijevog = 0, idDesnog = 0, indeksRoditelja = 0;
	int vrijednostLijevog = 0, vrijednostDesnog = 0;

	// najprije pronalazimo id lijevog i desnog djeteta početnog čvora
	for (i = 0; i < brojCvorova; i++) {
		if (privremeniCvorovi[i].id == idPocetnogCvora) {
			indeksRoditelja = i;
			idLijevog = privremeniCvorovi[i].lijevo;
			idDesnog = privremeniCvorovi[i].desno;
			break;
		}
	}

	// potom pronalazimo vrijednosti lijevog i desnog djeteta početnog čvora
	for (i = 0; i < brojCvorova; i++) {
		if (idLijevog != -1 && privremeniCvorovi[i].id == idLijevog) vrijednostLijevog = privremeniCvorovi[i].v;
		if (idDesnog != -1 && privremeniCvorovi[i].id == idDesnog) vrijednostDesnog = privremeniCvorovi[i].v;
	}

	// zatim stvaramo lijevi i desni čvor
	if (idLijevog != -1) stvoriLijevoB(vrijednostLijevog, indeksPocetnogCvora, stablo);
	if (idDesnog != -1) stvoriDesnoB(vrijednostDesnog, indeksPocetnogCvora, stablo);
	

	// postavljamo ID trenutnog čvora u polju privremenih čvorova na -1
	// to činimo kako bi kasnije mogli napraviti provjeru 
	// naime, ukoliko postoje privremeni čvorovi koji nisu iskorišteni, tada ćemo okinuti pogrešku
	privremeniCvorovi[indeksRoditelja].id = CVOR_ISKORISTEN;

	// i na kraju nastavljamo dalje puniti stablo za lijevi i desni čvor
	if (idLijevog != -1) popuniStablo(stablo, lijevoDijeteB(indeksPocetnogCvora, stablo), idLijevog, privremeniCvorovi, brojCvorova);
	if (idDesnog != -1) popuniStablo(stablo, desnoDijeteB(indeksPocetnogCvora, stablo), idDesnog, privremeniCvorovi, brojCvorova);
}



void provjeriNeiskoristeneCvorove(tmp_cvor *privremeniCvorovi, int brojCvorova) {
	int i;
	for (i = 0; i < brojCvorova; i++) {
		if (privremeniCvorovi[i].id != CVOR_ISKORISTEN) {
			pfree(privremeniCvorovi);

			ereport(ERROR,
				(errcode(ERRCODE_RAISE_EXCEPTION),
					errmsg("Neispravan unos stabla\n"))
			);

			return;
		}
	}
}