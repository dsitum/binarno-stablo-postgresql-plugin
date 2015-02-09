#define MAX_BROJ_CVOROVA 512 // binarno stablo na 8 hijerarhijskih razina
#define INDEKS_KORIJENA_STABLA 1
#define NEPOSTOJECI_CVOR 0

typedef struct bstablo bstablo;

/**
 * Memorijska reprezentacija čvora binarnog stabla
 *
 * v: vrijednost čvora stabla
 * iskoristen: daje informaciju o tome je li čvor iskorišten ili nije
 */
 struct bstablo {
 	int iskoristen;
 	int v;
 };



int roditeljB(int pozicija, bstablo T[]);
int lijevoDijeteB(int pozicijaRoditelja, bstablo T[]);
int desnoDijeteB(int pozicijaRoditelja, bstablo T[]);
int vrijednostB(int pozicija, bstablo T[]);
void promijeniVrijednostB(int v, int pozicija, bstablo T[]);
int korijenB(bstablo T[]);
void stvoriLijevoB(int v, int pozicijaRoditelja, bstablo T[]);
void stvoriDesnoB(int v, int pozicijaRoditelja, bstablo T[]);
void obrisiCvorB(int pozicija, bstablo T[]);
int jednakiB(bstablo T1[], bstablo T2[]);
bstablo *kopirajStabloB(bstablo *T);
bstablo *inicijalizirajB(int v);



/**
 * Vraća indeks polja roditelja čvora
 * @param  pozicija pozicija u polju od čvora kojem želimo naći indeks roditelja
 * @param  T        binarno stablo
 * @return          indeks polja roditelja stabla
 */
int roditeljB(int pozicija, bstablo T[]) {
	if ((int)(pozicija/2)) return pozicija/2;
	else return NEPOSTOJECI_CVOR;
}


/**
 * Vraća indeks lijevog djeteta čvora
 * @param  pozicijaRoditelja indeks polja roditelja čvora čije lijevo dijete želimo naći
 * @param  T                 binarno stablo
 * @return                   indeks lijevog djeteta čvora
 */
int lijevoDijeteB(int pozicijaRoditelja, bstablo T[]) {
	if (pozicijaRoditelja*2 >= MAX_BROJ_CVOROVA) {
		return NEPOSTOJECI_CVOR;
	} else if (T[pozicijaRoditelja*2].iskoristen) { 
		return pozicijaRoditelja*2;
	} else {
		return NEPOSTOJECI_CVOR;
	}
}


/**
 * Vraća indeks desnog djeteta čvora
 * @param  pozicijaRoditelja indeks polja roditelja čvora čije desno dijete želimo naći
 * @param  T                 binarno stablo
 * @return                   indeks desnog djeteta čvora
 */
int desnoDijeteB(int pozicijaRoditelja, bstablo T[]) {
	if (pozicijaRoditelja*2+1 >= MAX_BROJ_CVOROVA) {
		return NEPOSTOJECI_CVOR;
	} else if (T[pozicijaRoditelja*2+1].iskoristen) { 
		return pozicijaRoditelja*2+1;
	} else {
		return NEPOSTOJECI_CVOR;
	}
}


/**
 * Vraća vrijednost čvora
 * @param  pozicija indeks čvora u polju
 * @param  T        binarno stablo
 * @return          vrijednost čvora
 */
int vrijednostB(int pozicija, bstablo T[]) {
	if (T[pozicija].iskoristen==1) return T[pozicija].v;
	return NEPOSTOJECI_CVOR;
}


/**
 * Mijenja vrijednost čvora binarnog stabla
 * @param v        nova vrijednost čvora
 * @param pozicija indeks čvora u polju
 * @param T        binarno stablo
 */
void promijeniVrijednostB(int v, int pozicija, bstablo T[]) {
	T[pozicija].v=v;
}


/**
 * Daje odgovor na pitanje je li stablo prazno
 * @param  T binarno stablo
 * @return   Odgovor na pitanje je li stablo prazno
 */
int korijenB(bstablo T[]) {  //je li stablo prazno ?
	if (T[1].iskoristen) return 1;
	else return 0;
}


/**
 * Stvara lijevo dijete čvora
 * @param v                 vrijednost novostvorenog čvora
 * @param pozicijaRoditelja indeks polja roditelja stabla čije lijevo dijete želimo stvoriti
 * @param T                 binarno stablo
 */
void stvoriLijevoB(int v, int pozicijaRoditelja, bstablo T[]) {
	int lijevoDijete = pozicijaRoditelja*2;

	if (T[lijevoDijete].iskoristen) {
		ereport(ERROR,
			(errcode(ERRCODE_RAISE_EXCEPTION),
				errmsg("Taj cvor vec ima lijevo dijete!\n"))
		);
	} else {
		T[lijevoDijete].v=v;
		T[lijevoDijete].iskoristen=1;
	}
}


/**
 * Stvara desno dijete čvora
 * @param v                 vrijednost novostvorenog čvora
 * @param pozicijaRoditelja indeks polja roditelja stabla čije desno dijete želimo stvoriti
 * @param T                 binarno stablo
 */
void stvoriDesnoB(int v, int pozicijaRoditelja, bstablo T[]) {
	int desnoDijete = pozicijaRoditelja*2+1;

	if (T[desnoDijete].iskoristen) {
		ereport(ERROR,
			(errcode(ERRCODE_RAISE_EXCEPTION),
				errmsg("Taj cvor vec ima desno dijete!\n"))
		);
	} else {
		T[desnoDijete].v=v;
		T[desnoDijete].iskoristen=1;
	}
}


/**
 * Briše čvor i sve njegove potomke
 * @param pozicija indeks polja čvora koji želimo obrisati iz binarnog stabla
 * @param T        binarno stablo
 */
void obrisiCvorB(int pozicija, bstablo T[]) {
	if (pozicija < MAX_BROJ_CVOROVA && T[pozicija].iskoristen == 1) {
		T[pozicija].iskoristen=0;
		obrisiCvorB(lijevoDijeteB(pozicija, T), T);
		obrisiCvorB(desnoDijeteB(pozicija, T), T);
	}
}


/**
 * Provjerava jesu li dva binarna stabla jednaka
 * @param T1  prvo binarno stablo
 * @param T2  drugo binarno stablo
 * @return    ukoliko su binarna stabla jednaka, vraća 1. U suprotnom vraća 0
 */
int jednakiB(bstablo T1[], bstablo T2[]) {
	int i, jednaki = 1;

	for (i = INDEKS_KORIJENA_STABLA; i < MAX_BROJ_CVOROVA; i++) {
		if (T1[i].iskoristen != T2[i].iskoristen || T1[i].v != T2[i].v) {
			jednaki = 0;
			break;
		}
	}

	return jednaki;
}



/**
 * Obavlja duboko kopiranje stabla i vraća kopiju
 * @param  T Binarno stablo koje treba kopirati
 * @return   Duboka kopija binarnog stabla
 */
bstablo *kopirajStabloB(bstablo *T) {
	bstablo *T2 = palloc(MAX_BROJ_CVOROVA * sizeof(struct bstablo));
	memcpy(T2, T, MAX_BROJ_CVOROVA * sizeof(struct bstablo));
	return T2;
}


/**
 * Alocira binarno stablo u memoriji i inicijalizira njegov prvi čvor
 * @param  v  vrijednost korijena stabla
 * @return    Pokazivač na binarno stablo
 */
bstablo *inicijalizirajB(int v) {
	int i;

	bstablo *T = palloc(MAX_BROJ_CVOROVA * sizeof(struct bstablo));

	// postavljamo sve čvorove stabla na neiskorištene
	for (i = 0; i < MAX_BROJ_CVOROVA; i++) {
		T[i].iskoristen=0;
		T[i].v = 0;
	}

	// inicijaliziramo korijen stabla
	T[INDEKS_KORIJENA_STABLA].v=v;
	T[INDEKS_KORIJENA_STABLA].iskoristen=1;

	return T;
}