#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define MAX_LITERE_COMANDA 50
#define MAX_NUME_FISIER 100

typedef struct{
	double rosu;
	double verde;
	double albastru;
} matrice_de_numere_color;

typedef struct{
		char rosu[4];
		char verde[4];
		char albastru[4];
} matrice_color;

typedef struct{
	char numar_magic[3];
	char latime[10];
	char inaltime[10];
	char val_max[10];
	char ***matrice;
	matrice_color ***m;
	matrice_color ***copie;
	matrice_de_numere_color ***m_color;
	bool matrice_incarcata;
	bool matrice_color_incarcata;
} image;

void LOAD(image **, int *, int *, char[]);
int transformare(char[]);
void evitare_comentarii(FILE *);
void eliberare_matrice(int, int, image **);
void eliberare_matrice_color(int, int, image **);
void SELECT(image **, int[], int *, char[], int *, int *);
void SELECTALL(image **, int *, int[], int *, int *);
void HISTOGRAM(image **, int[], char[]);
void EQUALIZE(image **, int[]);
int clamp2(int, int, int);
void CROP(image **, int *, int *, int *, int *);
void APPLY(image **, int[], char[], int *,int *, int[]);
void SHARPEN(image **, int[], int *,int *, int[]);
void EDGE(image **, int[], int *, int *, int[]);
void BLUR(image **, int[], int *, int *, int[]);
void GAUSSIANBLUR(image **, int[], int *, int *, int[]);
void ROTATE(image **, int *, int[], char[], int *, int *);
void ROTATE_90_DREAPTA(image **, int *, int *, int *, int *);
void SAVE(image **, int[], char[]);
void EXIT(image **, int[]);
bool valid(int, int, int[]);

int main(void)
{
	char comanda[MAX_LITERE_COMANDA];
	char comanda1[MAX_LITERE_COMANDA];
	char comanda2[MAX_LITERE_COMANDA];
	char copie[MAX_LITERE_COMANDA];
	int dim[2];             // aici memorez dimensiunile originale
	int delimitator[2];    // aici memorez eventualele restrangeri ale imaginii
	int latime_start = 0;
	int inaltime_start = 0;
	image *x = (image *)malloc(sizeof(image));
	x->matrice_incarcata = false;
	x->matrice_color_incarcata = false;
	while (fgets(comanda, MAX_LITERE_COMANDA, stdin)) {
		strcpy(copie, comanda);  // pt a nu strica comanda
		char *del = strtok(copie, "\n ");
		int cnt = 0;
		while (del) {
			if (cnt == 0)
				strcpy(comanda1, del);
			else if (cnt == 1)
				strcpy(comanda2, del);
			cnt++;
			del = strtok(NULL, "\n ");
		}
		if (strcmp(comanda1, "LOAD") == 0) {
		LOAD(&x, dim, delimitator, comanda2);
		} else if (strcmp(comanda1, "SELECT") == 0 &&
			   	   strcmp(comanda2, "ALL") != 0) {
		SELECT(&x, dim, delimitator, comanda, &latime_start,
		   	   &inaltime_start);
		} else if (strcmp(comanda1, "SELECT") == 0 &&
			   	   strcmp(comanda2, "ALL") == 0) {
		SELECTALL(&x, delimitator, dim, &latime_start,
			  	  &inaltime_start);
		} else if (strcmp(comanda1, "HISTOGRAM") == 0) {
		HISTOGRAM(&x, dim, comanda);
		} else if (strcmp(comanda1, "EQUALIZE") == 0) {
		EQUALIZE(&x, dim);
		} else if (strcmp(comanda1, "CROP") == 0) {
		CROP(&x, delimitator, dim, &latime_start, &inaltime_start);
		} else if (strcmp(comanda1, "APPLY") == 0) {
		APPLY(&x, delimitator, comanda, &latime_start,
		  	  &inaltime_start, dim);
		} else if (strcmp(comanda1, "ROTATE") == 0) {
		ROTATE(&x, delimitator, dim, comanda, &latime_start,
		   	   &inaltime_start);
		} else if (strcmp(comanda1, "SAVE") == 0) {
		SAVE(&x, dim, comanda);
		} else if (strcmp(comanda1, "EXIT") == 0){
		EXIT(&x, delimitator);
		} else {
		printf("Invalid command\n");
		}
	}
	free(x);
	return 0;
}

void LOAD(image **x, int *dim, int *delimitator, char comanda2[])
{
	FILE *fisier;
	if (!(fisier = fopen(comanda2, "rt"))) {  // nu se deschide
		printf("Failed to load %s\n", comanda2);
		(*x)->matrice_incarcata = (*x)->matrice_color_incarcata = false;
	}	else {
		if ((*x)->matrice_incarcata)
			eliberare_matrice(dim[0], dim[1], x);
		// facem loc pt noua matrice
		if ((*x)->matrice_color_incarcata)
			eliberare_matrice_color(dim[0], dim[1], x);
		printf("Loaded %s\n", comanda2);
		char rest_enunt[150];  // pt a memora restul liniei deja citita partial
		fscanf(fisier, "%s", (*x)->numar_magic);
		fgets(rest_enunt, 150, fisier);
		evitare_comentarii(fisier);
		fscanf(fisier, "%s%s", (*x)->latime, (*x)->inaltime);
		fgets(rest_enunt, 150, fisier);
		evitare_comentarii(fisier);
		fscanf(fisier, "%s", (*x)->val_max);
		fgets(rest_enunt, 150, fisier);
		int latime = transformare((*x)->latime);
		int inaltime = transformare((*x)->inaltime);
		dim[0] = latime;
		delimitator[0] = latime;
		dim[1] = inaltime;
		delimitator[1] = inaltime;
		evitare_comentarii(fisier);
		int pozitie = ftell(fisier);
		fclose(fisier);

		if (strcmp((*x)->numar_magic, "P2") == 0) {
		fisier = fopen(comanda2, "rt"); // fisier text
		fseek(fisier, pozitie, SEEK_CUR);
		(*x)->matrice = (char ***)malloc(inaltime * sizeof(char **));
		for (int i = 0; i < inaltime; ++i)
			(*x)->matrice[i] = (char **)malloc(latime * sizeof(char *));
		for (int i = 0; i < inaltime; ++i)
			for (int j = 0; j < latime; ++j)
				(*x)->matrice[i][j] = (char *)malloc(4 * sizeof(char));
		for (int i = 0; i < inaltime; ++i)
			for (int j = 0; j < latime; ++j)
				fscanf(fisier, "%s", (*x)->matrice[i][j]);
		(*x)->matrice_incarcata = true; // actualizez existenta unei matrice
		fclose(fisier);

		} else if (strcmp((*x)->numar_magic, "P3") == 0) {
		fisier = fopen(comanda2, "rt");
		fseek(fisier, pozitie, SEEK_CUR);
		(*x)->m = (matrice_color ***)malloc
		(inaltime * sizeof(matrice_color **));
		for (int i = 0; i < inaltime; ++i)
			(*x)->m[i] = (matrice_color **)malloc
			(latime * sizeof(matrice_color *));
		for (int i = 0; i < inaltime; ++i)
			for (int j = 0; j < latime; ++j)
				(*x)->m[i][j] = (matrice_color *)malloc
				(4 * sizeof(matrice_color));
		for (int i = 0; i < inaltime; ++i)
			for (int j = 0; j < latime; ++j) {
				fscanf(fisier, "%s", (*x)->m[i][j]->rosu);
				fscanf(fisier, "%s", (*x)->m[i][j]->verde);
				fscanf(fisier, "%s", (*x)->m[i][j]->albastru);
		}
		(*x)->matrice_color_incarcata = true;
		fclose(fisier);

		} else if (strcmp((*x)->numar_magic, "P5") == 0) {
		fisier = fopen(comanda2, "rb"); // fisier binar
		fseek(fisier, pozitie, SEEK_CUR);
		(*x)->matrice = (char ***)malloc(inaltime * sizeof(char **));
		for (int i = 0; i < inaltime; ++i)
			(*x)->matrice[i] = (char **)malloc(latime * sizeof(char *));
		for (int i = 0; i < inaltime; ++i)
			for (int j = 0; j < latime; ++j)
				(*x)->matrice[i][j] = (char *)malloc(4 * sizeof(char));
		for (int i = 0; i < inaltime; ++i)
			for (int j = 0; j < latime; ++j) {
				unsigned char valoare;
				fread(&valoare, sizeof(unsigned char), 1, fisier);
				sprintf((*x)->matrice[i][j], "%u", valoare);
				// am folosit functia sprintf pt a incorpora un alt
				// tip de data intr un string
			}
		(*x)->matrice_incarcata = true;
		fclose(fisier);

		} else if (strcmp((*x)->numar_magic, "P6") == 0) {
		fisier = fopen(comanda2, "rb");
		fseek(fisier, pozitie, SEEK_CUR);
		(*x)->m = (matrice_color ***)malloc // pt canalele de culoare
		(inaltime * sizeof(matrice_color **));
		for (int i = 0; i < inaltime; ++i)
			(*x)->m[i] = (matrice_color **)malloc
			(latime * sizeof(matrice_color *));
		for (int i = 0; i < inaltime; ++i)
			for (int j = 0; j < latime; ++j)
				(*x)->m[i][j] = (matrice_color *)malloc
				(4 * sizeof(matrice_color));
		for (int i = 0; i < inaltime; ++i)
			for (int j = 0; j < latime; ++j) {
				unsigned char valoare1;
				fread(&valoare1, sizeof(unsigned char), 1, fisier);
				sprintf((*x)->m[i][j]->rosu, "%u", valoare1);
				unsigned char valoare2;
				fread(&valoare2, sizeof(unsigned char), 1, fisier);
				sprintf((*x)->m[i][j]->verde, "%u", valoare2);
				unsigned char valoare3;
				fread(&valoare3, sizeof(unsigned char), 1, fisier);
				sprintf((*x)->m[i][j]->albastru, "%u", valoare3);
		}
		(*x)->matrice_color_incarcata = true;
		fclose(fisier);
		}
	}
}

int transformare(char c[]) // transform stringurile in inturi
{
	int nr = 0;
	int cnt = 0;
	while (c[cnt] != '\0') {
		nr = nr * 10 + (c[cnt] - '0');
		cnt++;
	}
	return nr;
}

void evitare_comentarii(FILE *x)
//Citesc randuri pana cand ajung la unul valid. Atunci, dupa citirea randului
// valid, devansez nr de biti coresp liniei pt a face citirea adecvata
{
	char linie[150];
	while (fgets(linie, 150, x) != NULL) {
		if (linie[0] == '#' || linie[0] == '\n')
			continue;
		else {
			fseek(x, -strlen(linie), SEEK_CUR);
			break;
		}
	}
}

void eliberare_matrice(int latime, int inaltime, image **x)
{
	for (int i = 0; i < inaltime; ++i)
		for (int j = 0; j < latime; ++j)
			free((*x)->matrice[i][j]);
	for (int i = 0; i < inaltime; ++i)
		free((*x)->matrice[i]);
	free((*x)->matrice);
	(*x)->matrice_incarcata = false;
}

void eliberare_matrice_color(int latime, int inaltime, image **x)
{
	for (int i = 0; i < inaltime; ++i)
		for (int j = 0; j < latime; ++j)
			free((*x)->m[i][j]);
	for (int i = 0; i < inaltime; ++i)
		free((*x)->m[i]);
	free((*x)->m);
	(*x)->matrice_color_incarcata = false;
}

void SELECT(image **x, int dim[], int *delimitator, char comanda[],
			int *latime_start, int *inaltime_start)
{
	if (!(*x)->matrice_incarcata &&
		!(*x)->matrice_color_incarcata) {
		printf("No image loaded\n");
		return;
	}
	char x1c[10];
	char y1c[10];
	char x2c[10];
	char y2c[10];
	char *del = strtok(comanda, "\n ");
	int cnt = 0;
	while (del) {
	// am spart comanda in cuvinte si am atribuit valorile coresp
		if (cnt == 1)
			strcpy(x1c, del);
		else if (cnt == 2)
			strcpy(y1c, del);
		else if (cnt == 3)
			strcpy(x2c, del);
		else if (cnt == 4)
			strcpy(y2c, del);
		cnt++;
		del = strtok(NULL, "\n ");
	}

	// orice coordonata ce nu contine cifre sau semnul '-' reprezinta o comanda
	// invalida
	for (long unsigned int i = 0; i < strlen(x1c); ++i)
		if (((int)(x1c[i]) < 48 && (int)(x1c[i]) != 45) || (int)(x1c[i]) > 57) {
			printf("Invalid command\n");
			return;
		}
	for (long unsigned int i = 0 ; i < strlen(y1c); ++i)
		if (((int)(y1c[i]) < 48 && (int)(y1c[i]) != 45) || (int)(y1c[i]) > 57) {
			printf("Invalid command\n");
			return;
		}
	for (long unsigned int i = 0; i < strlen(x2c); ++i)
		if (((int)(x2c[i]) < 48 && (int)(x2c[i]) != 45) || (int)(x2c[i]) > 57) {
			printf("Invalid command\n");
			return;
		}
	for (long unsigned int i = 0; i < strlen(y2c); ++i)
		if (((int)(y2c[i]) < 48 && (int)(y2c[i]) != 45) || (int)(y2c[i]) > 57) {
			printf("Invalid command\n");
			return;
		}

	int x1 = transformare(x1c);
	int y1 = transformare(y1c);
	int x2 = transformare(x2c);
	int y2 = transformare(y2c);

	// interschimbam valorile in cazul oridinii neadecvate
	if (x1 > x2) {
		int aux = x1;
		x1 = x2;
		x2 = aux;
	}
	if (y1 > y2) {
		int aux = y1;
		y1 = y2;
		y2 = aux;
	}

	// verificam logica coordonatelor
	if (x1 < 0 || y1 < 0 || x2 > dim[0] || y2 > dim[1] || x1 == x2 ||
		y1 == y2) {
		printf("Invalid set of coordinates\n");
		return;
	}
	printf("Selected %d %d %d %d\n", x1, y1, x2, y2);
	delimitator[0] = x2 - x1; // actualizez noua dim (temporara)
	delimitator[1] = y2 - y1;
	*latime_start = x1; // aici memorez zona de inceput a selectiei pe latime
	*inaltime_start = y1; // si aici pe inaltime
}

void SELECTALL(image **x, int *delimitator, int dim[], int *latime_start,
			   int *inaltime_start)
{
	if (!(*x)->matrice_incarcata &&
		!(*x)->matrice_color_incarcata) {
		printf("No image loaded\n");
		return;
	}
	printf("Selected ALL\n");
	delimitator[0] = dim[0];
	delimitator[1] = dim[1];
	*latime_start = 0;
	*inaltime_start = 0;
}

void HISTOGRAM(image **x, int dim[], char comanda[])
{
	if (!(*x)->matrice_color_incarcata &&
		!(*x)->matrice_incarcata) {
		printf("No image loaded\n");
		return;
	}
	char xc[10];
	char yc[10];
	int cnt1 = 0;

	// verific corectitudinea comenzii
	if (comanda[strlen(comanda) - 1] == '\n')
		comanda[strlen(comanda) - 1] = '\0';
	if (strcmp(comanda, "HISTOGRAM") == 0) {
		printf("Invalid command\n");
		return;
	}
	char *del = strtok(comanda, "\n ");
	while (del) {
		if (cnt1 == 1)
			strcpy(xc, del);
		else if (cnt1 == 2)
			strcpy(yc, del);
		cnt1++;
		del = strtok(NULL, "\n ");
	}
	int X, Y;
	X = transformare(xc);
	Y = transformare(yc);

	// cnt1 == 4 --> Histograma contine 3 coordonate
	if (X == 0 || Y == 0 || cnt1 == 4) {
		printf("Invalid command\n");
		return;
	}
	if ((*x)->matrice_color_incarcata) {
		printf("Black and white image needed\n");
		return;
	}

	int *vector_frecventa = (int *)calloc(256, sizeof(int));
	int *frecventa_bin = (int *)calloc(256, sizeof(int));
	int *nr_stelute = (int *)malloc(256 * sizeof(int));

	for (int i = 0; i < dim[1]; ++i)
		for (int j = 0; j < dim[0]; ++j) {
			int nr = transformare((*x)->matrice[i][j]);
			vector_frecventa[nr]++; // frecventa fiecarui nr
		}

	int maxi = 0;
	int cnt = 0;
	int elem_per_bin = 256 / Y;

	// contorul creste si adaug fiecarui bin frecventa valorilor aferente
	for (int i = 0; i < Y; ++i) {
		for (int j = 0; j < elem_per_bin; ++j)
			frecventa_bin[i] += vector_frecventa[cnt++];
		if (frecventa_bin[i] > maxi)
			maxi = frecventa_bin[i];
	}
	for (int i = 0; i < Y; ++i)
		nr_stelute[i] = (int)(((double)frecventa_bin[i] / (double)maxi) * X);
	for (int i = 0; i < Y; ++i) {
		printf("%d\t|\t", nr_stelute[i]);
		for (int j = 0; j < nr_stelute[i]; ++j)
			printf("*");
		printf("\n");
	}
	free(vector_frecventa);
	free(frecventa_bin);
	free(nr_stelute);
}

void EQUALIZE(image **x, int dim[])
{
	if ((*x)->matrice_color_incarcata) {
		printf("Black and white image needed\n");
		return;
	}
	if (!(*x)->matrice_color_incarcata &&
		!(*x)->matrice_incarcata) {
		printf("No image loaded\n");
		return;
	}
	printf("Equalize done\n");
	int *vector_frecventa = (int *)calloc(256, sizeof(int));
	double arie = dim[0] * dim[1];
	for (int i = 0; i < dim[1]; ++i)
		for (int j = 0; j < dim[0]; ++j) {
			int nr = transformare((*x)->matrice[i][j]);
			vector_frecventa[nr]++;
		}
	for (int i = 0; i < dim[1]; ++i) {
		for (int j = 0; j < dim[0]; ++j) {
			double suma = 0;
			for (int l = 0; l <= transformare((*x)->matrice[i][j]); ++l) {
				suma += vector_frecventa[l];
				// adaug la suma frecventa valorilor mai mici sau egale decat
				// valoarea actuala
			}
			int nr = transformare((*x)->matrice[i][j]);
			double nr2 = (double)nr;
			nr2 = round((255.0 * suma) / arie);
			if (nr2 > 255) // compar valoarea cu valorile extreme
				nr2 = 255;
			else if (nr2 < 0)
				nr2 = 0;
			sprintf((*x)->matrice[i][j], "%.lf", nr2);
		}
	}
}

int clamp2(int min, int val_dorita, int max)
{
	if (val_dorita < min)
		return min;
	else if (val_dorita > max)
		return max;
	else
		return val_dorita;
}

void CROP(image **x, int *delimitator, int *dim, int *latime_start,
		  int *inaltime_start)
{
	if (!(*x)->matrice_incarcata &&
		!(*x)->matrice_color_incarcata) {
		printf("No image loaded\n");
		return;
	}
	printf("Image cropped\n");

	if ((*x)->matrice_incarcata) {
	char ***matrice_ajutatoare = (char ***)malloc
	(delimitator[1] * sizeof(char **));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_ajutatoare[i] = (char **)malloc
		(delimitator[0] * sizeof(char *));
	for (int i = 0; i < delimitator[1]; ++i)
		for (int j = 0; j < delimitator[0]; ++j)
			matrice_ajutatoare[i][j] = (char *)malloc
			(4 * sizeof(char));

	for (int i = 0; i < delimitator[1]; ++i)
		for (int j = 0; j < delimitator[0]; ++j)
			strcpy(matrice_ajutatoare[i][j], (*x)->matrice[*inaltime_start + i]
			[*latime_start + j]); // copiem elementele

	eliberare_matrice(dim[0], dim[1], x); // eliberam matricea initiala
	(*x)->matrice_incarcata = true;

	// alocam noul spatiu necesar, conform selectiei actuale
	(*x)->matrice = (char ***)malloc(delimitator[1] * sizeof(char **));
	for (int i = 0; i < delimitator[1]; ++i)
		(*x)->matrice[i] = (char **)malloc(delimitator[0] * sizeof(char *));
	for (int i = 0; i < delimitator[1]; ++i)
		for (int j = 0; j < delimitator[0]; ++j)
			(*x)->matrice[i][j] = (char *)malloc(4 * sizeof(char));

	// copiem elementele inapoi in matricea noastra
	for (int i = 0; i < delimitator[1]; ++i)
		for (int j = 0; j < delimitator[0]; ++j)
			strcpy((*x)->matrice[i][j], matrice_ajutatoare[i][j]);

	dim[0] = delimitator[0]; // actualizez noile dimensiuni
	dim[1] = delimitator[1];
	*latime_start = 0;
	*inaltime_start = 0;

	for (int i = 0; i < delimitator[1]; ++i)
		for (int j = 0; j < delimitator[0]; ++j)
			free(matrice_ajutatoare[i][j]);
	for (int i = 0; i < delimitator[1]; ++i)
		free(matrice_ajutatoare[i]);
	free(matrice_ajutatoare);
	}
	if ((*x)->matrice_color_incarcata) {
	(*x)->copie = (matrice_color ***)malloc
	(delimitator[1] * sizeof(matrice_color **));
	for (int i = 0; i < delimitator[1]; ++i)
		(*x)->copie[i] = (matrice_color **)malloc
		(delimitator[0] * sizeof(matrice_color *));
	for (int i = 0; i < delimitator[1]; ++i)
		for (int j = 0; j < delimitator[0]; ++j)
			(*x)->copie[i][j] = (matrice_color *)malloc
			(4 * sizeof(matrice_color));

	// copiem elementele
	for (int i = 0; i < delimitator[1]; ++i)
		for (int j = 0; j < delimitator[0]; ++j) {
			strcpy((*x)->copie[i][j]->rosu, (*x)->m[*inaltime_start + i]
			[*latime_start + j]->rosu);
			strcpy((*x)->copie[i][j]->verde, (*x)->m[*inaltime_start + i]
			[*latime_start + j]->verde);
			strcpy((*x)->copie[i][j]->albastru, (*x)->m[*inaltime_start + i]
			[*latime_start + j]->albastru);
		}

	eliberare_matrice_color(delimitator[0], delimitator[1], x);
	(*x)->matrice_color_incarcata = true;

	(*x)->m = (matrice_color ***)malloc
	(delimitator[1] * sizeof(matrice_color **));
	for (int i = 0; i < delimitator[1]; ++i)
		(*x)->m[i] = (matrice_color **)malloc
		(delimitator[0] * sizeof(matrice_color *));
	for (int i = 0; i < delimitator[1]; ++i)
		for (int j = 0; j < delimitator[0]; ++j)
			(*x)->m[i][j] = (matrice_color *)malloc
			(4 * sizeof(matrice_color));

	for (int i = 0; i < delimitator[1]; ++i)
		for (int j = 0; j < delimitator[0]; ++j){
			strcpy((*x)->m[i][j]->rosu, (*x)->copie[i][j]->rosu);
			strcpy((*x)->m[i][j]->verde, (*x)->copie[i][j]->verde);
			strcpy((*x)->m[i][j]->albastru, (*x)->copie[i][j]->albastru);
		}

	dim[0] = delimitator[0];
	dim[1] = delimitator[1];
	*latime_start = 0;
	*inaltime_start = 0;

	for (int i = 0; i < delimitator[1]; ++i)
		for (int j = 0; j < delimitator[0]; ++j)
			free((*x)->copie[i][j]);
	for (int i = 0; i < delimitator[1]; ++i)
		free((*x)->copie[i]);
	free((*x)->copie);
	}
}

void APPLY(image **x, int delimitator[], char comanda[], int *latime_start,
			int *inaltime_start, int dim[])
{
	if (!(*x)->matrice_incarcata &&
		!(*x)->matrice_color_incarcata) {
		printf("No image loaded\n");
		return;
	}
	if (comanda[strlen(comanda) - 1] == '\n')
		comanda[strlen(comanda) - 1] = '\0';
	if (strcmp(comanda, "APPLY") == 0) {
		printf("Invalid command\n");
		return;
	}
	char *del = strtok(comanda, "\n ");
	int cnt = 0;
	char optiune[MAX_LITERE_COMANDA];
	while (del) {
	// spargem comanda pt a memora optiunea dorita
		if (cnt == 1)
			strcpy(optiune, del);
		cnt++;
		del = strtok(NULL, "\n ");
	}
	if (strcmp(optiune, "EDGE") == 0) {
		EDGE(x, delimitator, latime_start, inaltime_start, dim);
		return;
	} else if (strcmp(optiune, "SHARPEN") == 0) {
		SHARPEN(x, delimitator, latime_start, inaltime_start, dim);
		return;
	} else if (strcmp(optiune, "BLUR") == 0) {
		BLUR(x, delimitator, latime_start, inaltime_start, dim);
		return;
	} else if (strcmp(optiune, "GAUSSIAN_BLUR") == 0) {
		GAUSSIANBLUR(x, delimitator, latime_start, inaltime_start, dim);
		return;
	} else {
		printf("APPLY parameter invalid\n");
		return;
	}
}

bool valid(int pozx, int pozy, int dim[])
// verifica daca o casuta este viabila pt aplicarea unui efect
{
	if (pozy < 1 || pozy > dim[1]-2 || pozx < 1 || pozx > dim[0] - 2)
		return false;
	return true;
}

void SHARPEN(image **x, int delimitator[], int *latime_start,
			 int *inaltime_start, int dim[])
{
	if ((*x)->matrice_incarcata) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}
	printf("APPLY SHARPEN done\n");

	// matricea kernel tip
	double kernel_matrix[3][3];
	kernel_matrix[0][0] = 0;
	kernel_matrix[0][1] = -1;
	kernel_matrix[0][2] = 0;
	kernel_matrix[1][0] = -1;
	kernel_matrix[1][1] = 5;
	kernel_matrix[1][2] = -1;
	kernel_matrix[2][0] = 0;
	kernel_matrix[2][1] = -1;
	kernel_matrix[2][2] = 0;

	// in aceasta matrice memoram valorile ca numere, nu ca stringuri
	(*x)->m_color = (matrice_de_numere_color ***)
	malloc((dim[1]) * sizeof(matrice_de_numere_color **));
		for (int i = 0; i < dim[1]; ++i)
			(*x)->m_color[i] = (matrice_de_numere_color **)malloc
			((dim[0]) * sizeof(matrice_de_numere_color *));
		for (int i = 0; i < dim[1]; ++i)
			for (int j = 0; j < dim[0]; ++j)
				(*x)->m_color[i][j] = (matrice_de_numere_color *)malloc
				(sizeof(matrice_de_numere_color));

	// facem transformarile
	for (int i = 0; i < dim[1]; ++i)
		for (int j = 0; j < dim[0]; ++j) {
			int rosu = transformare((*x)->m[i][j]->rosu);
			int verde = transformare((*x)->m[i][j]->verde);
			int albastru = transformare((*x)->m[i][j]->albastru);
			(*x)->m_color[i][j]->rosu = rosu;
			(*x)->m_color[i][j]->verde = verde;
			(*x)->m_color[i][j]->albastru = albastru;
	}

	// cu aceste matrici ajutatoare vom calcula noile valori
	int **matrice_ajutatoare_rosu = (int **)calloc
	(delimitator[1], sizeof(int *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_ajutatoare_rosu[i] = (int *)calloc
		(delimitator[0], sizeof(int));

	int **matrice_ajutatoare_verde = (int **)calloc
	(delimitator[1], sizeof(int *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_ajutatoare_verde[i] = (int *)calloc
		(delimitator[0], sizeof(int));

	int **matrice_ajutatoare_albastru = (int **)calloc
	(delimitator[1], sizeof(int *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_ajutatoare_albastru[i] = (int *)calloc
		(delimitator[0], sizeof(int));

	// verificam ce casute au fost supuse acestei modificari
	int **matrice_frecventa = (int **)calloc(delimitator[1], sizeof(int *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_frecventa[i] = (int *)calloc(delimitator[0], sizeof(int));

	for (int i = *inaltime_start; i < *inaltime_start + delimitator[1]; ++i) {
		for (int j = *latime_start; j < *latime_start + delimitator[0]; ++j) {
			if (valid(j, i, dim)) {
				// cand o casuta este valida, actualizez si matricea de
				// frecventa
				matrice_frecventa[i - *inaltime_start][j - *latime_start] = 1;
				for (int k = i - 1; k < i + 2; ++k) { // aici realizez calculele
					for (int p = j - 1; p < j + 2; ++p) {
						matrice_ajutatoare_rosu[i - *inaltime_start]
						[j - *latime_start] += ((*x)->m_color[k][p]->rosu *
						kernel_matrix[k - (i - 1)][p - (j - 1)]);
						matrice_ajutatoare_verde[i - *inaltime_start]
						[j - *latime_start] += ((*x)->m_color[k][p]->verde *
						kernel_matrix[k - (i - 1)][p - (j - 1)]);
						matrice_ajutatoare_albastru[i - *inaltime_start]
						[j - *latime_start] += ((*x)->m_color[k][p]->albastru *
						kernel_matrix[k - (i - 1)][p - (j - 1)]);
					}
				}
					matrice_ajutatoare_rosu[i - *inaltime_start]
					[j - *latime_start] = clamp2(0, matrice_ajutatoare_rosu
					[i - *inaltime_start][j - *latime_start], 255);
					matrice_ajutatoare_verde[i - *inaltime_start]
					[j - *latime_start] = clamp2(0, matrice_ajutatoare_verde
					[i - *inaltime_start][j - *latime_start], 255);
					matrice_ajutatoare_albastru[i - *inaltime_start]
					[j - *latime_start] = clamp2(0, matrice_ajutatoare_albastru
					[i - *inaltime_start][j - *latime_start], 255);
			}
		}
	}

	// actualizez numerele supuse modificarii si in matricea originala
	for (int i = *inaltime_start; i < *inaltime_start + delimitator[1]; ++i)
		for (int j = *latime_start; j < *latime_start + delimitator[0]; ++j) {
			if (matrice_frecventa[i - *inaltime_start][j - *latime_start]) {
				sprintf((*x)->m[i][j]->rosu, "%d", matrice_ajutatoare_rosu
				[i - *inaltime_start][j - *latime_start]);
				sprintf((*x)->m[i][j]->verde, "%d", matrice_ajutatoare_verde
				[i - *inaltime_start][j - *latime_start]);
				sprintf((*x)->m[i][j]->albastru, "%d",
				matrice_ajutatoare_albastru[i - *inaltime_start]
				[j - *latime_start]);
			}
		}

	for (int i = 0; i < dim[1]; ++i)
		for (int j = 0; j < dim[0]; ++j)
			free((*x)->m_color[i][j]);
	for (int i = 0; i < dim[1]; ++i)
		free((*x)->m_color[i]);
	free((*x)->m_color);

	for (int i = 0; i < delimitator[1]; ++i) {
		free(matrice_ajutatoare_rosu[i]);
		free(matrice_ajutatoare_verde[i]);
		free(matrice_ajutatoare_albastru[i]);
		free(matrice_frecventa[i]);
	}
	free(matrice_ajutatoare_rosu);
	free(matrice_ajutatoare_verde);
	free(matrice_ajutatoare_albastru);
	free(matrice_frecventa);
}

void EDGE(image **x, int delimitator[], int *latime_start, int *inaltime_start,
		  int dim[])
{
	if ((*x)->matrice_incarcata) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}
	printf("APPLY EDGE done\n");
	double kernel_matrix[3][3];
	kernel_matrix[0][0] = -1;
	kernel_matrix[0][1] = -1;
	kernel_matrix[0][2] = -1;
	kernel_matrix[1][0] = -1;
	kernel_matrix[1][1] = 8;
	kernel_matrix[1][2] = -1;
	kernel_matrix[2][0] = -1;
	kernel_matrix[2][1] = -1;
	kernel_matrix[2][2] = -1;

	(*x)->m_color = (matrice_de_numere_color ***)malloc
	(dim[1] * sizeof(matrice_de_numere_color **));
		for (int i = 0; i < dim[1]; ++i)
			(*x)->m_color[i] = (matrice_de_numere_color **)malloc
			(dim[0] * sizeof(matrice_de_numere_color *));
		for (int i = 0; i < dim[1]; ++i)
			for (int j = 0; j < dim[0]; ++j)
				(*x)->m_color[i][j] = (matrice_de_numere_color *)malloc
				(sizeof(matrice_de_numere_color));

	for (int i = 0; i < dim[1]; ++i)
		for (int j = 0; j < dim[0]; ++j) {
			int rosu = transformare((*x)->m[i][j]->rosu);
			int verde = transformare((*x)->m[i][j]->verde);
			int albastru = transformare((*x)->m[i][j]->albastru);
			(*x)->m_color[i][j]->rosu = rosu;
			(*x)->m_color[i][j]->verde = verde;
			(*x)->m_color[i][j]->albastru = albastru;
	}

	int **matrice_ajutatoare_rosu = (int **)calloc
	(delimitator[1], sizeof(int *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_ajutatoare_rosu[i] = (int *)calloc
		(delimitator[0], sizeof(int));

	int **matrice_ajutatoare_verde = (int **)calloc
	(delimitator[1], sizeof(int *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_ajutatoare_verde[i] = (int *)calloc
		(delimitator[0], sizeof(int));

	int **matrice_ajutatoare_albastru = (int **)calloc
	(delimitator[1], sizeof(int *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_ajutatoare_albastru[i] = (int *)calloc
		(delimitator[0], sizeof(int));

	int **matrice_frecventa = (int **)calloc
	(delimitator[1], sizeof(int *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_frecventa[i] = (int *)calloc
		(delimitator[0], sizeof(int));

	for (int i = *inaltime_start; i < *inaltime_start + delimitator[1]; ++i) {
		for (int j = *latime_start; j < *latime_start + delimitator[0]; ++j) {
			if (valid(j, i, dim)) {
				matrice_frecventa[i - *inaltime_start][j - *latime_start] = 1;
				for (int k = i - 1; k < i + 2; ++k) {
					for (int p = j - 1; p < j + 2; ++p) {
						matrice_ajutatoare_rosu[i - *inaltime_start]
						[j - *latime_start] += ((*x)->m_color[k][p]->rosu *
						kernel_matrix[k - (i - 1)][p - (j - 1)]);
						matrice_ajutatoare_verde[i - *inaltime_start]
						[j - *latime_start] += ((*x)->m_color[k][p]->verde *
						kernel_matrix[k - (i - 1)][p - (j - 1)]);
						matrice_ajutatoare_albastru[i - *inaltime_start]
						[j - *latime_start] += ((*x)->m_color[k][p]->albastru *
						kernel_matrix[k - (i - 1)][p - (j - 1)]);
					}
				}
					// functia clamp2 ofera siguranta unei valori adecvate
					matrice_ajutatoare_rosu[i - *inaltime_start]
					[j - *latime_start] = clamp2(0, matrice_ajutatoare_rosu
					[i - *inaltime_start][j - *latime_start], 255);
					matrice_ajutatoare_verde[i - *inaltime_start]
					[j - *latime_start] = clamp2(0, matrice_ajutatoare_verde
					[i - *inaltime_start][j - *latime_start], 255);
					matrice_ajutatoare_albastru[i - *inaltime_start]
					[j - *latime_start] = clamp2(0, matrice_ajutatoare_albastru
					[i - *inaltime_start][j - *latime_start], 255);
			}
		}
	}
	for (int i = *inaltime_start; i < *inaltime_start + delimitator[1]; ++i)
		for (int j = *latime_start; j < *latime_start + delimitator[0]; ++j) {
			if (matrice_frecventa[i - *inaltime_start][j - *latime_start]) {
				sprintf((*x)->m[i][j]->rosu, "%d", matrice_ajutatoare_rosu
				[i - *inaltime_start][j - *latime_start]);
				sprintf((*x)->m[i][j]->verde, "%d", matrice_ajutatoare_verde
				[i - *inaltime_start][j - *latime_start]);
				sprintf((*x)->m[i][j]->albastru, "%d", matrice_ajutatoare_albastru
				[i - *inaltime_start][j - *latime_start]);
			}
		}

	for (int i = 0; i < dim[1]; ++i)
		for (int j = 0; j < dim[0]; ++j)
			free((*x)->m_color[i][j]);
	for (int i = 0; i < dim[1]; ++i)
		free((*x)->m_color[i]);
	free((*x)->m_color);

	for (int i = 0; i < delimitator[1]; ++i) {
		free(matrice_ajutatoare_rosu[i]);
		free(matrice_ajutatoare_verde[i]);
		free(matrice_ajutatoare_albastru[i]);
		free(matrice_frecventa[i]);
	}
	free(matrice_ajutatoare_rosu);
	free(matrice_ajutatoare_verde);
	free(matrice_ajutatoare_albastru);
	free(matrice_frecventa);
}

void BLUR(image **x, int delimitator[], int *latime_start, int *inaltime_start,
		int dim[])
{
	if ((*x)->matrice_incarcata) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}
	printf("APPLY BLUR done\n");
	double kernel_matrix[3][3];
	kernel_matrix[0][0] = 1.0 / 9.0;
	kernel_matrix[0][1] = 1.0 / 9.0;
	kernel_matrix[0][2] = 1.0 / 9.0;
	kernel_matrix[1][0] = 1.0 / 9.0;
	kernel_matrix[1][1] = 1.0 / 9.0;
	kernel_matrix[1][2] = 1.0 / 9.0;
	kernel_matrix[2][0] = 1.0 / 9.0;
	kernel_matrix[2][1] = 1.0 / 9.0;
	kernel_matrix[2][2] = 1.0 / 9.0;

	(*x)->m_color = (matrice_de_numere_color ***)malloc
	(dim[1] * sizeof(matrice_de_numere_color **));
		for (int i = 0; i < dim[1]; ++i)
			(*x)->m_color[i] = (matrice_de_numere_color **)malloc
			(dim[0] * sizeof(matrice_de_numere_color *));
		for (int i = 0; i < dim[1]; ++i)
			for (int j = 0; j < dim[0]; ++j)
				(*x)->m_color[i][j] = (matrice_de_numere_color *)malloc
				(sizeof(matrice_de_numere_color));

	for (int i = 0; i < dim[1]; ++i)
		for (int j = 0; j < dim[0]; ++j) {
			int rosu = transformare((*x)->m[i][j]->rosu);
			int verde = transformare((*x)->m[i][j]->verde);
			int albastru = transformare((*x)->m[i][j]->albastru);
			(*x)->m_color[i][j]->rosu = rosu;
			(*x)->m_color[i][j]->verde = verde;
			(*x)->m_color[i][j]->albastru = albastru;
	}

	double **matrice_ajutatoare_rosu = (double **)calloc
	(delimitator[1], sizeof(double *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_ajutatoare_rosu[i] = (double *)calloc
		(delimitator[0], sizeof(double));

	double **matrice_ajutatoare_verde = (double **)calloc
	(delimitator[1], sizeof(double *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_ajutatoare_verde[i] = (double *)calloc
		(delimitator[0], sizeof(double));

	double **matrice_ajutatoare_albastru = (double **)calloc
	(delimitator[1], sizeof(double *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_ajutatoare_albastru[i] = (double *)calloc
		(delimitator[0], sizeof(double));

	int **matrice_frecventa = (int **)calloc
	(delimitator[1], sizeof(int *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_frecventa[i] = (int *)calloc
		(delimitator[0], sizeof(int));

	for (int i = *inaltime_start; i < *inaltime_start + delimitator[1]; ++i) {
		for (int j = *latime_start; j < *latime_start + delimitator[0]; ++j) {
			if (valid(j, i, dim)){
				matrice_frecventa[i - *inaltime_start][j - *latime_start] = 1;
				for (int k = i - 1; k < i + 2; ++k) {
					for (int p = j - 1; p < j + 2; ++p) {
						matrice_ajutatoare_rosu[i - *inaltime_start]
						[j - *latime_start] += ((*x)->m_color[k][p]->rosu *
						kernel_matrix[k - (i - 1)][p - (j - 1)]);
						matrice_ajutatoare_verde[i - *inaltime_start]
						[j - *latime_start] += ((*x)->m_color[k][p]->verde *
						kernel_matrix[k - (i - 1)][p - (j - 1)]);
						matrice_ajutatoare_albastru[i - *inaltime_start]
						[j - *latime_start] += ((*x)->m_color[k][p]->albastru *
						kernel_matrix[k - (i -1)][p - (j - 1)]);
					}
				}
					matrice_ajutatoare_rosu[i - *inaltime_start]
					[j - *latime_start] = clamp2(0, matrice_ajutatoare_rosu
					[i - *inaltime_start][j - *latime_start], 255);
					matrice_ajutatoare_verde[i - *inaltime_start]
					[j - *latime_start] = clamp2(0, matrice_ajutatoare_verde
					[i - *inaltime_start][j - *latime_start], 255);
					matrice_ajutatoare_albastru[i - *inaltime_start]
					[j - *latime_start] = clamp2(0, matrice_ajutatoare_albastru
					[i - *inaltime_start][j - *latime_start], 255);
					matrice_ajutatoare_rosu[i - *inaltime_start]
					[j - *latime_start] = round(matrice_ajutatoare_rosu
					[i - *inaltime_start][j - *latime_start]);
					matrice_ajutatoare_verde[i - *inaltime_start]
					[j - *latime_start] = round(matrice_ajutatoare_verde
					[i - *inaltime_start][j - *latime_start]);
					matrice_ajutatoare_albastru[i - *inaltime_start]
					[j - *latime_start] = round(matrice_ajutatoare_albastru
					[i - *inaltime_start][j - *latime_start]);
			}
		}
	}
	for (int i = *inaltime_start; i < *inaltime_start + delimitator[1]; ++i)
		for (int j = *latime_start; j < *latime_start + delimitator[0]; ++j) {
			if (matrice_frecventa[i - *inaltime_start][j - *latime_start]) {
				sprintf((*x)->m[i][j]->rosu, "%.lf", matrice_ajutatoare_rosu
				[i - *inaltime_start][j - *latime_start]);
				sprintf((*x)->m[i][j]->verde, "%.lf", matrice_ajutatoare_verde
				[i - *inaltime_start][j - *latime_start]);
				sprintf((*x)->m[i][j]->albastru, "%.lf",
				matrice_ajutatoare_albastru[i - *inaltime_start]
				[j - *latime_start]);
			}
		}

	for (int i = 0 ; i < dim[1]; ++i)
		for (int j = 0; j < dim[0]; ++j)
			free((*x)->m_color[i][j]);
	for (int i = 0; i < dim[1]; ++i)
		free((*x)->m_color[i]);
	free((*x)->m_color);

	for (int i = 0; i < delimitator[1]; ++i){
		free(matrice_ajutatoare_rosu[i]);
		free(matrice_ajutatoare_verde[i]);
		free(matrice_ajutatoare_albastru[i]);
		free(matrice_frecventa[i]);
	}
	free(matrice_ajutatoare_rosu);
	free(matrice_ajutatoare_verde);
	free(matrice_ajutatoare_albastru);
	free(matrice_frecventa);
}

void GAUSSIANBLUR(image **x, int delimitator[], int *latime_start,
				  int *inaltime_start, int dim[])
{
	if ((*x)->matrice_incarcata) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}
	printf("APPLY GAUSSIAN_BLUR done\n");
	double kernel_matrix[3][3];
	kernel_matrix[0][0] = 1.0 / 16.0;
	kernel_matrix[0][1] = 2.0 / 16.0;
	kernel_matrix[0][2] = 1.0 / 16.0;
	kernel_matrix[1][0] = 2.0 / 16.0;
	kernel_matrix[1][1] = 4.0 / 16.0;
	kernel_matrix[1][2] = 2.0 / 16.0;
	kernel_matrix[2][0] = 1.0 / 16.0;
	kernel_matrix[2][1] = 2.0 / 16.0;
	kernel_matrix[2][2] = 1.0 / 16.0;

	(*x)->m_color = (matrice_de_numere_color ***)malloc
	(dim[1] * sizeof(matrice_de_numere_color **));
		for (int i = 0; i < dim[1]; ++i)
			(*x)->m_color[i] = (matrice_de_numere_color **)malloc
			(dim[0] * sizeof(matrice_de_numere_color *));
		for (int i = 0; i < dim[1]; ++i)
			for (int j = 0; j < dim[0]; ++j)
				(*x)->m_color[i][j] = (matrice_de_numere_color *)malloc
				(sizeof(matrice_de_numere_color));

	for (int i = 0; i < dim[1]; ++i)
		for (int j = 0; j < dim[0]; ++j) {
			int rosu = transformare((*x)->m[i][j]->rosu);
			int verde = transformare((*x)->m[i][j]->verde);
			int albastru = transformare((*x)->m[i][j]->albastru);
			(*x)->m_color[i][j]->rosu = rosu;
			(*x)->m_color[i][j]->verde = verde;
			(*x)->m_color[i][j]->albastru = albastru;
	}

	double **matrice_ajutatoare_rosu = (double **)calloc
	(delimitator[1], sizeof(double *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_ajutatoare_rosu[i] = (double *)calloc
		(delimitator[0], sizeof(double));

	double **matrice_ajutatoare_verde = (double **)calloc
	(delimitator[1], sizeof(double *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_ajutatoare_verde[i] = (double *)calloc
		(delimitator[0], sizeof(double));

	double **matrice_ajutatoare_albastru = (double **)calloc
	(delimitator[1], sizeof(double *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_ajutatoare_albastru[i] = (double *)calloc
		(delimitator[0], sizeof(double));

	int **matrice_frecventa = (int **)calloc
	(delimitator[1], sizeof(int *));
	for (int i = 0; i < delimitator[1]; ++i)
		matrice_frecventa[i] = (int *)calloc
		(delimitator[0], sizeof(int));

	for (int i = *inaltime_start; i < *inaltime_start + delimitator[1]; ++i) {
		for (int j = *latime_start; j < *latime_start + delimitator[0]; ++j) {
			if (valid(j, i, dim)) {
				matrice_frecventa[i - *inaltime_start][j - *latime_start] = 1;
				for (int k = i - 1; k < i + 2; ++k) {
					for (int p = j - 1; p < j + 2; ++p) {
						matrice_ajutatoare_rosu[i - *inaltime_start]
						[j - *latime_start] += ((*x)->m_color[k][p]->rosu *
						kernel_matrix[k - (i - 1)][p - (j - 1)]);
						matrice_ajutatoare_verde[i - *inaltime_start]
						[j - *latime_start] += ((*x)->m_color[k][p]->verde *
						kernel_matrix[k - (i - 1)][p - (j - 1)]);
						matrice_ajutatoare_albastru[i - *inaltime_start]
						[j - *latime_start] += ((*x)->m_color[k][p]->albastru *
						kernel_matrix[k - (i - 1)][p - (j - 1)]);
					}
				}
					matrice_ajutatoare_rosu[i - *inaltime_start]
					[j - *latime_start] = clamp2(0, matrice_ajutatoare_rosu
					[i - *inaltime_start][j - *latime_start], 255);
					matrice_ajutatoare_verde[i - *inaltime_start]
					[j - *latime_start] = clamp2(0, matrice_ajutatoare_verde
					[i - *inaltime_start][j - *latime_start], 255);
					matrice_ajutatoare_albastru[i - *inaltime_start]
					[j - *latime_start] = clamp2(0, matrice_ajutatoare_albastru
					[i - *inaltime_start][j - *latime_start], 255);
					matrice_ajutatoare_rosu[i - *inaltime_start]
					[j - *latime_start] = round(matrice_ajutatoare_rosu
					[i - *inaltime_start][j - *latime_start]);
					matrice_ajutatoare_verde[i - *inaltime_start]
					[j - *latime_start] = round(matrice_ajutatoare_verde
					[i - *inaltime_start][j - *latime_start]);
					matrice_ajutatoare_albastru[i - *inaltime_start]
					[j - *latime_start] = round(matrice_ajutatoare_albastru
					[i - *inaltime_start][j - *latime_start]);
			}
		}
	}
	for (int i = *inaltime_start; i < *inaltime_start + delimitator[1]; ++i)
		for (int j = *latime_start; j < *latime_start + delimitator[0]; ++j) {
			if (matrice_frecventa[i - *inaltime_start][j - *latime_start]) {
				sprintf((*x)->m[i][j]->rosu, "%.lf", matrice_ajutatoare_rosu
				[i - *inaltime_start][j - *latime_start]);
				sprintf((*x)->m[i][j]->verde, "%.lf", matrice_ajutatoare_verde
				[i - *inaltime_start][j - *latime_start]);
				sprintf((*x)->m[i][j]->albastru, "%.lf",
				matrice_ajutatoare_albastru[i - *inaltime_start]
				[j - *latime_start]);
			}
		}

	for (int i = 0; i < dim[1]; ++i)
		for (int j = 0; j < dim[0]; ++j)
			free((*x)->m_color[i][j]);
	for (int i = 0; i < dim[1]; ++i)
		free((*x)->m_color[i]);
	free((*x)->m_color);

	for (int i = 0; i < delimitator[1]; ++i) {
		free(matrice_ajutatoare_rosu[i]);
		free(matrice_ajutatoare_verde[i]);
		free(matrice_ajutatoare_albastru[i]);
		free(matrice_frecventa[i]);
	}
	free(matrice_ajutatoare_rosu);
	free(matrice_ajutatoare_verde);
	free(matrice_ajutatoare_albastru);
	free(matrice_frecventa);
}

void ROTATE(image **x, int *delimitator, int dim[], char comanda[],
			int *latime_start, int *inaltime_start)
{
	if (!(*x)->matrice_incarcata &&
		!(*x)->matrice_color_incarcata) {
		printf("No image loaded\n");
		return;
	}
	if ((delimitator[0] != delimitator[1]) && (delimitator[0] != dim[0] ||
		delimitator[1] != dim[1])) {
		printf("The selection must be square\n");
		// daca au fost efectuate selectii, dar acestea nu sunt patratice
		return;
	}
	char unghic[10];
	char *del = strtok(comanda, "\n ");
	int cnt = 0;
	while (del) {
		if (cnt == 1)
			strcpy(unghic, del);
		cnt++;
		del = strtok(NULL, "\n ");
	}
	int unghi;
	if (unghic[0] == '-') { // <=> unghiul este negativ
		for (long unsigned int i = 0; i < strlen(unghic) - 1; ++i)
			unghic[i] = unghic[i + 1];
		unghic[strlen(unghic) - 1] = '\0';
		// efetuam tranformarea pe caractere, mai putin '-', iar mai apoi
		// schimbam semnul
		unghi = -(transformare(unghic));
	}	else
		unghi = transformare(unghic);

	// verific corectitudinea unghiurilor
	if ((abs)(unghi) != 90 && (abs)(unghi) != 180 && (abs)(unghi) != 270 &&
		(abs)(unghi) != 360 && unghi != 0) {
		printf("Unsupported rotation angle\n");
		return;
	}
	printf("Rotated %d\n", unghi);

	// am implementat o functie care roteste matricea cu 90 de grade
	// si am utilizat o prin repetitie pt a o roti la orice grad
	if (unghi > 0) {
		int mutari = unghi / 90;
		for (int i = 0; i < 4 - mutari; ++i)
			ROTATE_90_DREAPTA(x, delimitator, dim, latime_start,
			inaltime_start);
	} else {
		int mutari = (abs)(unghi) / 90;
		for (int i = 0; i < mutari; ++i)
			ROTATE_90_DREAPTA(x, delimitator, dim, latime_start,
			inaltime_start);
	}
}

void ROTATE_90_DREAPTA(image **x, int *delimitator, int *dim,
						int *latime_start, int *inaltime_start)
{
	if (delimitator[0] != delimitator[1]) {
		int aux = delimitator[0];
		delimitator[0] = delimitator[1];
		delimitator[1] = aux;
		int aux2 = dim[0];
		dim[0] = dim[1];
		dim[1] = aux2;
	}
	if ((*x)->matrice_incarcata) {
		// facem o copie cu dimensiunile adecvate
		char ***copie = (char ***)malloc(delimitator[1] * sizeof(char **));
		for (int i = 0; i < delimitator[1]; ++i)
			copie[i] = (char **)malloc(delimitator[0] * sizeof(char *));
		for (int i = 0; i < delimitator[1]; ++i)
			for (int j = 0; j < delimitator[0]; ++j)
				copie[i][j] = (char *)malloc(4 * sizeof(char));

		// introduc in copie elementele continute de selectie
		for (int i = delimitator[1] - 1; i >= 0; --i)
			for (int j = 0; j < delimitator[0]; ++j)
				strcpy(copie[delimitator[1] - i - 1][j], (*x)->matrice
				[j + *inaltime_start][i + *latime_start]);

		// in acest caz eliberam matricea, ii alocam noul spatiu
		// si efectuam rotirea
		if (delimitator[0] != delimitator[1]) {
			eliberare_matrice(delimitator[1], delimitator[0], x);
			(*x)->matrice_incarcata = true;
			(*x)->matrice = (char ***)malloc(delimitator[1] * sizeof(char **));
			for (int i = 0; i < delimitator[1]; ++i)
				(*x)->matrice[i] = (char **)malloc
				(delimitator[0] * sizeof(char *));
			for (int i = 0; i < delimitator[1]; ++i)
				for (int j = 0; j < delimitator[0]; ++j)
					(*x)->matrice[i][j] = (char *)malloc(4 * sizeof(char));
			for (int i = *inaltime_start; i < delimitator[1] +
				*inaltime_start; ++i)
				for (int j = *latime_start; j < delimitator[0] +
				*latime_start; ++j)
					strcpy((*x)->matrice[i][j], copie[i - *inaltime_start]
					[j - *latime_start]);
		}
		else {
			for (int i = *inaltime_start; i < delimitator[1] +
				*inaltime_start; ++i)
				for (int j = *latime_start; j < delimitator[0] +
					*latime_start; ++j)
					strcpy((*x)->matrice[i][j], copie[i - *inaltime_start]
					[j - *latime_start]);
		}
		for (int i = 0; i < delimitator[1]; ++i)
			for (int j = 0; j < delimitator[0]; ++j)
				free(copie[i][j]);
		for (int i = 0; i < delimitator[1]; ++i)
			free(copie[i]);
		free(copie);

	} else if ((*x)->matrice_color_incarcata) {
		(*x)->copie = (matrice_color ***)malloc
		(delimitator[1] * sizeof(matrice_color **));
		for (int i = 0; i < delimitator[1]; ++i)
			(*x)->copie[i] = (matrice_color **)malloc
			(delimitator[0] * sizeof(matrice_color *));
		for (int i = 0; i < delimitator[1]; ++i)
			for (int j = 0; j < delimitator[0]; ++j)
				(*x)->copie[i][j] = (matrice_color *)malloc
				(4 * sizeof(matrice_color));
		for (int i = delimitator[1] - 1; i >= 0; --i) {
			for (int j = 0; j < delimitator[0]; ++j) {
				strcpy((*x)->copie[delimitator[1] - i - 1][j]->rosu,
				(*x)->m[j + *inaltime_start][i + *latime_start]->rosu);
				strcpy((*x)->copie[delimitator[1] - i - 1][j]->verde,
				(*x)->m[j + *inaltime_start][i + *latime_start]->verde);
				strcpy((*x)->copie[delimitator[1] - i - 1][j]->albastru,
				(*x)->m[j + *inaltime_start][i + *latime_start]->albastru);
			}
		}
		if (delimitator[0] != delimitator[1]) {
			eliberare_matrice_color(delimitator[1], delimitator[0], x);
			(*x)->matrice_color_incarcata = true;
			(*x)->m = (matrice_color ***)malloc
			(delimitator[1] * sizeof(matrice_color **));
			for (int i = 0; i < delimitator[1]; ++i)
				(*x)->m[i] = (matrice_color **)malloc
				(delimitator[0] * sizeof(matrice_color *));
			for (int i = 0; i < delimitator[1]; ++i)
				for (int j = 0; j < delimitator[0]; ++j)
					(*x)->m[i][j] = (matrice_color *)malloc
					(4 * sizeof(matrice_color));
			for (int i = *inaltime_start; i < delimitator[1] +
				*inaltime_start; ++i)
				for (int j = *latime_start; j < delimitator[0] +
					*latime_start; ++j){
					strcpy((*x)->m[i][j]->rosu, (*x)->copie
					[i - *inaltime_start][j - *latime_start]->rosu);
					strcpy((*x)->m[i][j]->verde, (*x)->copie
					[i - *inaltime_start][j - *latime_start]->verde);
					strcpy((*x)->m[i][j]->albastru, (*x)->copie
					[i - *inaltime_start][j - *latime_start]->albastru);
				}
		}
		else {
			for (int i = *inaltime_start; i < delimitator[1] +
				*inaltime_start; ++i)
				for (int j = *latime_start; j < delimitator[0] +
					*latime_start; ++j) {
					strcpy((*x)->m[i][j]->rosu, (*x)->copie
					[i - *inaltime_start][j - *latime_start]->rosu);
					strcpy((*x)->m[i][j]->verde, (*x)->copie
					[i - *inaltime_start][j - *latime_start]->verde);
					strcpy((*x)->m[i][j]->albastru, (*x)->copie
					[i - *inaltime_start][j - *latime_start]->albastru);
				}
		}
		for (int i = 0; i < delimitator[1]; ++i)
			for (int j = 0; j < delimitator[0]; ++j)
				free((*x)->copie[i][j]);
		for (int i = 0; i < delimitator[1]; ++i)
			free((*x)->copie[i]);
		free((*x)->copie);
		}
}

void SAVE(image **x, int dim[], char comanda[])
{
	int cnt = 0;
	char nume_fisier_destinatie[MAX_NUME_FISIER];
	char tip_salvare[MAX_NUME_FISIER];
	char *del = strtok(comanda, "\n ");
	while (del) {
	// memoram tipul salvarii dorite
		if (cnt == 1)
			strcpy(nume_fisier_destinatie, del);
		else if (cnt == 2)
			strcpy(tip_salvare, del);
		cnt++;
		del = strtok(NULL, "\n ");
	}
	if ((*x)->matrice_incarcata == false &&
		(*x)->matrice_color_incarcata == false) {
		printf("No image loaded\n");
		return;
	}
	// tipul ascii
	if (strcmp(tip_salvare, "ascii") == 0) {
		FILE *fisierout = fopen(nume_fisier_destinatie, "wt");
		printf("Saved %s\n", nume_fisier_destinatie);
		if ((*x)->matrice_incarcata) {
			strcpy((*x)->numar_magic, "P2");
			fprintf(fisierout, "%s\n", (*x)->numar_magic);
			sprintf((*x)->latime, "%d", dim[0]);
			sprintf((*x)->inaltime, "%d", dim[1]);
			fprintf(fisierout, "%s %s\n", (*x)->latime, (*x)->inaltime);
			fprintf(fisierout, "%s\n", (*x)->val_max);
			for (int i = 0; i < dim[1]; ++i, fprintf(fisierout, "\n"))
				for (int j = 0; j < dim[0]; ++j)
					fprintf(fisierout, "%s ", (*x)->matrice[i][j]);
		}
		else if ((*x)->matrice_color_incarcata) {
			strcpy((*x)->numar_magic, "P3");
			fprintf(fisierout, "%s\n", (*x)->numar_magic);
			sprintf((*x)->latime, "%d", dim[0]);
			sprintf((*x)->inaltime, "%d", dim[1]);
			fprintf(fisierout, "%s %s\n", (*x)->latime, (*x)->inaltime);
			fprintf(fisierout, "%s\n", (*x)->val_max);
			for (int i = 0; i < dim[1]; ++i, fprintf(fisierout, "\n"))
				for (int j = 0; j < dim[0]; ++j){
					fprintf(fisierout, "%s ", (*x)->m[i][j]->rosu);
					fprintf(fisierout, "%s ", (*x)->m[i][j]->verde);
					fprintf(fisierout, "%s ", (*x)->m[i][j]->albastru);
			}
		}
		fclose(fisierout);
	} else { // tipul binar
		FILE *fisierout = fopen(nume_fisier_destinatie, "wb");
		printf("Saved %s\n", nume_fisier_destinatie);
		if ((*x)->matrice_incarcata) {
			strcpy((*x)->numar_magic, "P5");
			fprintf(fisierout, "%s\n", (*x)->numar_magic);
			sprintf((*x)->latime, "%d", dim[0]);
			sprintf((*x)->inaltime, "%d", dim[1]);
			fprintf(fisierout, "%s %s\n", (*x)->latime, (*x)->inaltime);
			fprintf(fisierout, "%s\n", (*x)->val_max);
			for (int i = 0; i < dim[1]; ++i)
				for (int j = 0; j < dim[0]; ++j) {
					int valoare = transformare((*x)->matrice[i][j]);
					unsigned char valoare2 = (unsigned char)(valoare);
					fwrite(&valoare2, sizeof(unsigned char), 1, fisierout);
			}
		}
		else if ((*x)->matrice_color_incarcata) {
			strcpy((*x)->numar_magic, "P6");
			fprintf(fisierout, "%s\n", (*x)->numar_magic);
			sprintf((*x)->latime, "%d", dim[0]);
			sprintf((*x)->inaltime, "%d", dim[1]);
			fprintf(fisierout, "%s %s\n", (*x)->latime, (*x)->inaltime);
			fprintf(fisierout, "%s\n", (*x)->val_max);
			for (int i = 0; i < dim[1]; ++i)
				for (int j = 0; j < dim[0]; ++j) {
					int valoare1 = transformare((*x)->m[i][j]->rosu);
					unsigned char valoare11 = (unsigned char)(valoare1);
					fwrite(&valoare11, sizeof(unsigned char), 1, fisierout);
					int valoare2 = transformare((*x)->m[i][j]->verde);
					unsigned char valoare22 = (unsigned char)(valoare2);
					fwrite(&valoare22, sizeof(unsigned char), 1, fisierout);
					int valoare3 = transformare((*x)->m[i][j]->albastru);
					unsigned char valoare33 = (unsigned char)(valoare3);
					fwrite(&valoare33, sizeof(unsigned char), 1, fisierout);
			}
		}
		fclose(fisierout);
	}
}

void EXIT(image **x, int delimitator[])
// eliberam resursele folosite si inchidem programul
{
	if (!(*x)->matrice_incarcata &&
		!(*x)->matrice_color_incarcata) {
		printf("No image loaded\n");
		return;
	}
	if ((*x)->matrice_incarcata)
		eliberare_matrice(delimitator[0], delimitator[1], x);
	else if ((*x)->matrice_color_incarcata)
		eliberare_matrice_color(delimitator[0], delimitator[1], x);
	exit(0);
}
