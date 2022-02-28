/********************************************************
 * Projekat: Kosilica na travnjaku
 * Namena:   Prvi predmetni zadatak
 * Modul:    main.c
 * Autor:    Dusan Zderic AI 24/2018
*******************************************************/
#include <SGF.h>  // zaglavlje za rad sa grafickim prozorom
#define WISINA_TRAVNJAKA -WINDOW_HEIGHT/9
#define SKRACENJE 700
#define NEDOGLED_X 0
#define NEDOGLED_Y WISINA_TRAVNJAKA
#define INITIALIZE_MODEL { .NumOfVerts = 0,.NumOfQuads = 0, .NumOfTris = 0, .NumOfLines = 0, .ImaPravac = 0, .BoundingRadious = 0}
#define INITIALIZE_SCENE { .BrojModela = 0, .BrojInstancimodela = 0, .BrojStvariZaSortiranje = 0}
#define INITIALIZE_INSTANCE {.scale = 1, .Rotate.x = 0, .Rotate.y = 0, .Rotate.z = 0}
#define SHOW_PIVOT 1

enum Poly
{
	Tri,
	Quad
};
enum Lights
{
	PointLight,
	DirectionalLight,
	AmbientLight
};


struct Tacka2D
{
	int x;
	int y;
};
struct Tacka3D
{
	int x;
	int y;
	int z;
};
struct Vektor3D
{
	float x;
	float y;
	float z;
};

struct Vektor2D
{
	float x;
	float y;
};

struct Vertex
{
	struct Vektor3D pozicija;
	struct Tacka2D ScreenSpacePos;
};

struct QuadPoly
{
	int vertcies[4];
	struct Vektor3D normala;
	struct Material* material;
	float UdaljenostOdKamere; //ako je negativan ovaj broj onda ne gleda ka kameri.
};

struct TriPoly
{
	int vertcies[3];
	struct Vektor3D normala;
	struct Material* material;
	float UdaljenostOdKamere;
};

struct Line
{
	int vertcies[2];
	int debljina;
	TColor boja;
	float UdaljenostOdKamere;
};

struct Kamera
{
	struct Vektor3D Pozicija;
	struct Vektor3D Pravac;
	int skracenje;
};

struct Model
{
	struct Vertex* Vertexptr;
	int NumOfVerts;

	float BoundingRadious;

	struct QuadPoly* QuadPtr;
	int NumOfQuads;

	struct TriPoly* TriPtr;
	int NumOfTris;

	struct Line* LinePtr;
	int NumOfLines;

	struct Vektor3D Pivot;
	struct Vektor3D Pravac;
	int ImaPravac;

	char naziv[3];
};

struct ModelInstance
{
	struct Model* ModelReference;
	struct Vektor3D Translate;
	float scale;
	struct Vektor3D Rotate;
};

struct Material
{
	float temp_intensity;
	float roughness;
	int metalic;
	struct Vektor3D boja;

};

struct Light
{
	enum Lights lightype;
	float intensity;
	struct Vektor3D Pozicija_Pravac;
	struct Vektor3D boja;
};

struct ModelListNode
{
	struct Model* ModelScene;
	struct ModelListNode* PtrSledecegModela;
};

struct ModelINstanceListNode
{
	struct ModelInstance* ModelScene;
	struct ModelINstanceListNode* PtrSledecegModela;
};

struct LightListNode
{
	struct ModelInstance* SvetloScene;
	struct LightListNode* PtrSledecegSvetla;
};

struct Scena
{
	struct ModelListNode ListaModela;
	int BrojModela;

	struct ModelINstanceListNode ListaInstanciModela;
	int BrojInstancimodela;

	struct LightListNode ListaSvetla;
	int BrojSvetla;

	int BrojStvariZaSortiranje;
	int BrojTempVerteksa;

	struct Kamera Kamera;
};

struct TwoIntsAndFloat
{
	int x;
	int y;
	float z;
};

//////////////////////////////////misc functions//////////////////////////////////////

float DuzinaTacke2D(struct Tacka2D Vektor2d)
{
	return sqrt((float)(Vektor2d.x * Vektor2d.x + Vektor2d.y * Vektor2d.y));
}

float DuzinaVektora3D(struct Vektor3D Vektor3d)
{
	return sqrt((float)(Vektor3d.x * Vektor3d.x + Vektor3d.y * Vektor3d.y + Vektor3d.z * Vektor3d.z));
}

float DuzinaVektora3DBezKorenovanja(struct Vektor3D Vektor3d)
{
	return Vektor3d.x * Vektor3d.x + Vektor3d.y * Vektor3d.y + Vektor3d.z * Vektor3d.z;
}

void NormalizeVektor3D(struct Vektor3D* Vektor3d)
{
	float duzina = sqrt((float)(Vektor3d->x * Vektor3d->x + Vektor3d->y * Vektor3d->y + Vektor3d->z * Vektor3d->z));
	Vektor3d->x = Vektor3d->x / duzina;
	Vektor3d->y = Vektor3d->y / duzina;
	Vektor3d->z = Vektor3d->z / duzina;
}

void NormalizeVektor2D(struct Vektor2D* Vektor3d)
{
	float duzina = sqrt((float)(Vektor3d->x * Vektor3d->x + Vektor3d->y * Vektor3d->y));
	Vektor3d->x = Vektor3d->x / duzina;
	Vektor3d->y = Vektor3d->y / duzina;
}

int DuzinaTacke3DBezKorenovanja(struct Tacka3D Vektor3d)
{
	return Vektor3d.x * Vektor3d.x + Vektor3d.y * Vektor3d.y + Vektor3d.z * Vektor3d.z;
}

float DuzinaFloatova3DBezKorenovanja(float x, float y, float z)
{
	return (x * x) + (y * y) + (z * z);
}

float SkracenjeUOdnosuNaZ(int Zkoordinata, int skracenje)
{
	return 1.0 / pow(2.0, (float)Zkoordinata / (float)skracenje);
}


float Inrerpolate(float min, float max, float alpha)
{
	return (max - min) * alpha + min;
}


float DegreeToRadians(float ugao)
{
	return (ugao * 3.142) / 180;
}


//////////////////////////////////////quick sort//////////////////////////////////////////////

void swap(struct TwoIntsAndFloat* a, struct TwoIntsAndFloat* b)
{
	struct TwoIntsAndFloat c = *a;
	*a = *b;
	*b = c;
}

int partition(struct TwoIntsAndFloat Input[], int low, int high)
{
	int pivot = Input[high].z;
	int i = low;

	for (int j = low; j <= high - 1; j++)
	{
		if (Input[j].z >= pivot)
		{
			swap(&Input[i], &Input[j]);
			i++;
		}
	}
	swap(&Input[i], &Input[high]);
	return (i);
}
void quickSort(struct TwoIntsAndFloat Input[], int low, int high)
{
	if (low < high)
	{
		int pi = partition(Input, low, high);

		quickSort(Input, low, pi - 1);
		quickSort(Input, pi + 1, high);
	}
}



//////////////////////////////////////2D funkcije/////////////////////////////////////////////


struct Tacka2D NapraviTacku2D(int x, int y)
{
	struct Tacka2D povratnaTacka;
	povratnaTacka.x = x;
	povratnaTacka.y = y;
	return povratnaTacka;
}

struct Tacka2D SrednjaTacka2D(int brojTacaka, int* tacka)
{
	struct Tacka2D CentralnaTacka;
	int xSum = 0;
	int ySum = 0;
	for (int i = 0; i < brojTacaka; i++)
	{
		xSum += *tacka++;
		ySum += *tacka++;
	}

	CentralnaTacka.x = xSum / brojTacaka;
	CentralnaTacka.y = ySum / brojTacaka;
	return CentralnaTacka;
}

////////////////////////////////////////////transformacije 2D//////////////////////////////////////////////

void translate2D(int brojTacaka, int* tacka, int Xtransf, int Ytransf)
{
	for (int i = 0; i < brojTacaka; i++)
	{
		*tacka++ += Xtransf;
		*tacka++ += Ytransf;
	}
}

void Scale2D(int brojTacaka, int* tacka, struct Tacka2D TackaSkaliranja, float kolicina)
{
	int* TempPointerToStart = tacka;
	translate2D(brojTacaka, TempPointerToStart, -TackaSkaliranja.x, -TackaSkaliranja.y);
	for (int i = 0; i < brojTacaka; i++)
	{
		*tacka++ *= kolicina;
		*tacka++ *= kolicina;
	}
	translate2D(brojTacaka, TempPointerToStart, TackaSkaliranja.x, TackaSkaliranja.y);
}

void Rotate2D(int brojTacaka, int* tacka, struct Tacka2D TackaRotacije, float angeloInRdiano)
{
	int TempX, TempY;
	int* TempPointerToStart = tacka;
	translate2D(brojTacaka, TempPointerToStart, -TackaRotacije.x, -TackaRotacije.y);

	for (int i = 0; i < brojTacaka; i++)
	{
		TempX = *tacka++;
		TempY = *tacka--;
		*tacka++ = TempX * cos(angeloInRdiano) + TempY * sin(angeloInRdiano);
		*tacka++ = TempY * cos(angeloInRdiano) - TempX * sin(angeloInRdiano);
	}
	translate2D(brojTacaka, TempPointerToStart, TackaRotacije.x, TackaRotacije.y);
}


///////////////////////////////////////3D funkcije////////////////////////////////////////////


struct Tacka3D NapraviTacku3D(int x, int y, int z)
{
	struct Tacka3D povratnaTacka;
	povratnaTacka.x = x;
	povratnaTacka.y = y;
	povratnaTacka.z = z;
	return povratnaTacka;
}


struct Tacka3D SrednjaTacka3D(int brojTacaka, int* tacka)
{
	struct Tacka3D CentralnaTacka;
	int xSum = 0;
	int ySum = 0;
	int zSum = 0;
	for (int i = 0; i < brojTacaka; i++)
	{
		xSum += *tacka++;
		ySum += *tacka++;
		zSum += *tacka++;
	}

	CentralnaTacka.x = xSum / brojTacaka;
	CentralnaTacka.y = ySum / brojTacaka;
	CentralnaTacka.z = zSum / brojTacaka;
	return CentralnaTacka;
}

struct Tacka3D Saberi3DTacke(struct Tacka3D A, struct Tacka3D B)
{
	struct Tacka3D C = { .x = A.x + B.x,.y = A.y + B.y,.z = A.z + B.z };
	return C;
}



struct Vektor3D NapraviVektor3D(float x, float y, float z)
{
	struct Vektor3D povratnaTacka;
	povratnaTacka.x = x;
	povratnaTacka.y = y;
	povratnaTacka.z = z;
	return povratnaTacka;
}

struct Vektor2D NapraviVektor2D(float x, float y)
{
	struct Vektor2D povratnaTacka;
	povratnaTacka.x = x;
	povratnaTacka.y = y;
	return povratnaTacka;
}

void RotateVectorAroundY3D(struct Vektor3D* Vektor, float Ugao)
{
	struct Vektor3D TempVerteks;

	TempVerteks = *Vektor;
	(*Vektor).x = TempVerteks.x*cos(Ugao) - TempVerteks.z * sin(Ugao);
	(*Vektor).z = TempVerteks.x*sin(Ugao) + TempVerteks.z * cos(Ugao);
}

struct Vektor3D SaberiVector3D(struct Vektor3D vektor1, struct Vektor3D vektor2)
{
	vektor1.x += vektor2.x;
	vektor1.y += vektor2.y;
	vektor1.z += vektor2.z;
	return vektor1;
}

struct Vektor3D OduzmiVector3D(struct Vektor3D vektor1, struct Vektor3D vektor2)
{
	vektor1.x -= vektor2.x;
	vektor1.y -= vektor2.y;
	vektor1.z -= vektor2.z;
	return vektor1;
}


void RotateVectorAroundY3DByVector(struct Vektor3D* Vektor, struct Vektor3D Pravac)
{
	struct Vektor3D TempVerteks;
	struct Vektor2D Pravac2D;

	Pravac2D = NapraviVektor2D(Pravac.x, Pravac.z);
	NormalizeVektor2D(&Pravac2D);


	TempVerteks = *Vektor;
	(*Vektor).x = TempVerteks.x * Pravac2D.y - TempVerteks.z * Pravac2D.x;
	(*Vektor).z = TempVerteks.z * Pravac2D.y + TempVerteks.x * Pravac2D.x;
}
////////////////////////////////////////////srednje vrednosti poligona i linija///////////////////////////////////////////////////////

struct Vektor3D SrednjiVerteksQuada(struct Vertex* NizTacaka, struct QuadPoly* Polygon)
{
	struct Vektor3D Prosek =
	{
	.x = (NizTacaka[Polygon->vertcies[0]].pozicija.x + NizTacaka[Polygon->vertcies[1]].pozicija.x + NizTacaka[Polygon->vertcies[2]].pozicija.x + NizTacaka[Polygon->vertcies[3]].pozicija.x) / 4,
	.y = (NizTacaka[Polygon->vertcies[0]].pozicija.y + NizTacaka[Polygon->vertcies[1]].pozicija.y + NizTacaka[Polygon->vertcies[2]].pozicija.y + NizTacaka[Polygon->vertcies[3]].pozicija.y) / 4,
	.z = (NizTacaka[Polygon->vertcies[0]].pozicija.z + NizTacaka[Polygon->vertcies[1]].pozicija.z + NizTacaka[Polygon->vertcies[2]].pozicija.z + NizTacaka[Polygon->vertcies[3]].pozicija.z) / 4
	};
	return Prosek;
}

struct Vektor3D SrednjiVerteksTria(struct Vertex* NizTacaka, struct TriPoly* Polygon)
{
	struct Vektor3D Prosek =
	{
	.x = (NizTacaka[Polygon->vertcies[0]].pozicija.x + NizTacaka[Polygon->vertcies[1]].pozicija.x + NizTacaka[Polygon->vertcies[2]].pozicija.x) / 3,
	.y = (NizTacaka[Polygon->vertcies[0]].pozicija.y + NizTacaka[Polygon->vertcies[1]].pozicija.y + NizTacaka[Polygon->vertcies[2]].pozicija.y) / 3,
	.z = (NizTacaka[Polygon->vertcies[0]].pozicija.z + NizTacaka[Polygon->vertcies[1]].pozicija.z + NizTacaka[Polygon->vertcies[2]].pozicija.z) / 3
	};
	return Prosek;
}

struct Vektor3D SrednjiVerteksLinije(struct Vertex* NizTacaka, struct Line* Polygon)
{
	struct Vektor3D Prosek =
	{
	.x = (NizTacaka[Polygon->vertcies[0]].pozicija.x + NizTacaka[Polygon->vertcies[1]].pozicija.x) / 2,
	.y = (NizTacaka[Polygon->vertcies[0]].pozicija.y + NizTacaka[Polygon->vertcies[1]].pozicija.y) / 2,
	.z = (NizTacaka[Polygon->vertcies[0]].pozicija.z + NizTacaka[Polygon->vertcies[1]].pozicija.z) / 2
	};
	return Prosek;
}



///////////////////////////////////////////////transformacije Modela////////////////////////////////////////////

void TranslateModel3D(struct Model* Model, struct Vektor3D pomeraj)
{
	for (int i = 0; i < Model->NumOfVerts; i++)
	{
		Model->Vertexptr[i].pozicija.x += pomeraj.x;
		Model->Vertexptr[i].pozicija.y += pomeraj.y;
		Model->Vertexptr[i].pozicija.z += pomeraj.z;
	}
	Model->Pivot.x += pomeraj.x;
	Model->Pivot.y += pomeraj.y;
	Model->Pivot.z += pomeraj.z;
}

void Scale3D(struct Model* Model, float kolicina)
{
	struct Vektor3D PivotOriginal = Model->Pivot;
	Model->BoundingRadious *= kolicina;


	TranslateModel3D(&Model, NapraviVektor3D(-Model->Pivot.x, -Model->Pivot.y, -Model->Pivot.z));
	for (int i = 0; i < Model->NumOfVerts; i++)
	{
		Model->Vertexptr[i].pozicija.x *= kolicina;
		Model->Vertexptr[i].pozicija.y *= kolicina;
		Model->Vertexptr[i].pozicija.z *= kolicina;
	}
	TranslateModel3D(&Model, PivotOriginal);
}

void RotateAroundY3D(struct Model* Model, float Ugao)
{
	struct Vektor3D PivotOriginal = Model->Pivot;
	struct Vektor3D TempVerteks;

	TranslateModel3D(Model, NapraviVektor3D(-Model->Pivot.x, -Model->Pivot.y, -Model->Pivot.z));

	for (int i = 0; i < Model->NumOfVerts; i++)
	{
		TempVerteks = Model->Vertexptr[i].pozicija;
		Model->Vertexptr[i].pozicija.x = TempVerteks.x*cos(Ugao) - TempVerteks.z * sin(Ugao);
		Model->Vertexptr[i].pozicija.z = TempVerteks.x*sin(Ugao) + TempVerteks.z * cos(Ugao);
	}
	if (Model->ImaPravac)
	{
		TempVerteks = Model->Pravac;
		Model->Pravac.x = TempVerteks.x*cos(Ugao) - TempVerteks.z * sin(Ugao);
		Model->Pravac.z = TempVerteks.x*sin(Ugao) + TempVerteks.z * cos(Ugao);
		NormalizeVektor3D(&Model->Pravac);
	}

	TranslateModel3D(Model, PivotOriginal);
}


//////////////////////////////////////////scene functions///////////////////////////////////////////

//Funkcije za Modele



void AddModelToScene(struct Scena* _Scena, struct Model* _Model)
{
	if (_Scena->BrojModela == 0)
	{
		_Scena->ListaModela.ModelScene = _Model;
		_Scena->ListaModela.PtrSledecegModela = NULL;
		_Scena->BrojModela = 1;
		_Scena->BrojTempVerteksa = _Model->NumOfVerts;
	}
	else {
		struct ModelListNode* Lista = &(_Scena->ListaModela);
		while (Lista->PtrSledecegModela != NULL)
		{
			Lista = Lista->PtrSledecegModela;
		}
		Lista->PtrSledecegModela = malloc(sizeof(struct ModelListNode));
		if (Lista->PtrSledecegModela == NULL)
		{
			printf("rip");
		}

		Lista->PtrSledecegModela->ModelScene = _Model;
		Lista->PtrSledecegModela->PtrSledecegModela = NULL;
		_Scena->BrojModela++;
		if (_Scena->BrojTempVerteksa < _Model->NumOfVerts) _Scena->BrojTempVerteksa = _Model->NumOfVerts;
	}
	//cuva najveci broj elemenata koje trebaju da se sortiraju zbog manje alokacije memorije pri crtanju
	if (_Scena->BrojStvariZaSortiranje < (_Model->NumOfQuads + _Model->NumOfTris + _Model->NumOfLines)) _Scena->BrojStvariZaSortiranje = _Model->NumOfQuads + _Model->NumOfTris + _Model->NumOfLines;
}

struct Model* GetModelFromScene(struct Scena* _Scena, int indeks)
{
	struct ModelListNode* Lista = &(_Scena->ListaModela);
	for (int i = 0; i < indeks; i++)
	{
		Lista = Lista->PtrSledecegModela;
	}
	return Lista->ModelScene;
}

//funkcije za istance modela



void AddInstanceToScene(struct Scena* _Scena, struct ModelInstance* _ModelInst)
{
	if (_Scena->BrojInstancimodela == 0)
	{
		_Scena->ListaInstanciModela.ModelScene = _ModelInst;
		_Scena->ListaInstanciModela.PtrSledecegModela = NULL;
		_Scena->BrojInstancimodela = 1;

	}
	else {
		struct ModelINstanceListNode* Lista = &(_Scena->ListaInstanciModela);
		while (Lista->PtrSledecegModela != NULL)
		{
			Lista = Lista->PtrSledecegModela;
		}
		Lista->PtrSledecegModela = malloc(sizeof(struct ModelListNode));

		if (Lista->PtrSledecegModela == NULL)
		{
			printf("rip");
		}

		Lista->PtrSledecegModela->ModelScene = _ModelInst;
		Lista->PtrSledecegModela->PtrSledecegModela = NULL;
		_Scena->BrojInstancimodela++;
	}

}

struct ModelInstance* GetModelInstaneFromScene(struct Scena* _Scena, int indeks)
{
	struct ModelINstanceListNode* Lista = &(_Scena->ListaInstanciModela);
	for (int i = 0; i < indeks; i++)
	{
		Lista = Lista->PtrSledecegModela;
	}
	return Lista->ModelScene;
}

//funkcije za svetla scene

void AddLightToScene(struct Scena* _Scena, struct Light* _SvetloListe)
{
	if (_Scena->BrojSvetla == 0)
	{
		_Scena->ListaSvetla.SvetloScene = _SvetloListe;
		_Scena->ListaSvetla.PtrSledecegSvetla = NULL;
		_Scena->BrojSvetla = 1;

	}
	else {
		struct LightListNode* Lista = &(_Scena->ListaSvetla);
		while (Lista->PtrSledecegSvetla != NULL)
		{
			Lista = Lista->PtrSledecegSvetla;
		}
		Lista->PtrSledecegSvetla = malloc(sizeof(struct LightListNode));

		if (Lista->PtrSledecegSvetla == NULL)
		{
			printf("rip");
		}

		Lista->PtrSledecegSvetla->SvetloScene = _SvetloListe;
		Lista->PtrSledecegSvetla->PtrSledecegSvetla = NULL;
		_Scena->BrojSvetla++;
	}

}

struct Light* GetLightFromScene(struct Scena* _Scena, int indeks)
{
	struct LightListNode* Lista = &(_Scena->ListaSvetla);
	for (int i = 0; i < indeks; i++)
	{
		Lista = Lista->PtrSledecegSvetla;
	}
	return Lista->SvetloScene;
}


void FreeScene(struct Scena* _Scena)
{


	struct ModelListNode* ModelList;
	struct ModelListNode* PredhodnaModelList;
	while (_Scena->BrojModela > 1)
	{
		ModelList = _Scena->ListaModela.PtrSledecegModela;
		PredhodnaModelList = &_Scena->ListaModela;
		while (ModelList->PtrSledecegModela != NULL)
		{
			ModelList = ModelList->PtrSledecegModela;
			PredhodnaModelList = PredhodnaModelList->PtrSledecegModela;
		}
		free(ModelList);
		PredhodnaModelList->PtrSledecegModela = NULL;
		_Scena->BrojModela--;
	}

	struct ModelINstanceListNode* InstanceList;
	struct ModelINstanceListNode* PredhodnaInstanceList;
	while (_Scena->BrojModela > 1)
	{
		InstanceList = _Scena->ListaModela.PtrSledecegModela;
		PredhodnaInstanceList = &_Scena->ListaModela;
		while (InstanceList->PtrSledecegModela != NULL)
		{
			InstanceList = InstanceList->PtrSledecegModela;
			PredhodnaInstanceList = PredhodnaInstanceList->PtrSledecegModela;
		}
		free(InstanceList);
		PredhodnaInstanceList->PtrSledecegModela = NULL;
		_Scena->BrojModela--;
	}

	struct LightListNode* LightList;
	struct LightListNode* PredhodnoSvetloList;
	while (_Scena->BrojModela > 1)
	{
		LightList = _Scena->ListaSvetla.PtrSledecegSvetla;
		PredhodnoSvetloList = &_Scena->ListaSvetla;
		while (LightList->PtrSledecegSvetla != NULL)
		{
			LightList = LightList->PtrSledecegSvetla;
			PredhodnoSvetloList = PredhodnoSvetloList->PtrSledecegSvetla;
		}
		free(LightList);
		PredhodnoSvetloList->PtrSledecegSvetla = NULL;
		_Scena->BrojSvetla--;
	}
}


///////////////////////////////////////////render functions///////////////////////////////////////////

TColor ProznadjiBoju(struct Vektor3D boja)
{
	struct Vektor3D SortPoJednomIndeksu;
	//red
	SortPoJednomIndeksu.y = DuzinaFloatova3DBezKorenovanja(boja.x - 1, boja.y, boja.z);
	SortPoJednomIndeksu.z = RED;
	//lime
	SortPoJednomIndeksu.x = DuzinaFloatova3DBezKorenovanja(boja.x, boja.y - 1, boja.z);
	if (SortPoJednomIndeksu.x < SortPoJednomIndeksu.y)
	{
		SortPoJednomIndeksu.y = SortPoJednomIndeksu.x;
		SortPoJednomIndeksu.z = LIME;
	}
	//blue
	SortPoJednomIndeksu.x = DuzinaFloatova3DBezKorenovanja(boja.x, boja.y, boja.z - 1);
	if (SortPoJednomIndeksu.x < SortPoJednomIndeksu.y)
	{
		SortPoJednomIndeksu.y = SortPoJednomIndeksu.x;
		SortPoJednomIndeksu.z = BLUE;
	}
	//green
	SortPoJednomIndeksu.x = DuzinaFloatova3DBezKorenovanja(boja.x, boja.y - 0.5, boja.z);
	if (SortPoJednomIndeksu.x < SortPoJednomIndeksu.y)
	{
		SortPoJednomIndeksu.y = SortPoJednomIndeksu.x;
		SortPoJednomIndeksu.z = GREEN;
	}
	//pink
	SortPoJednomIndeksu.x = DuzinaFloatova3DBezKorenovanja(boja.x - 1, boja.y - 0.5, boja.z - 1);
	if (SortPoJednomIndeksu.x < SortPoJednomIndeksu.y)
	{
		SortPoJednomIndeksu.y = SortPoJednomIndeksu.x;
		SortPoJednomIndeksu.z = PINK;
	}
	//brown
	SortPoJednomIndeksu.x = DuzinaFloatova3DBezKorenovanja(boja.x - 0.6, boja.y - 0.18, boja.z);
	if (SortPoJednomIndeksu.x < SortPoJednomIndeksu.y)
	{
		SortPoJednomIndeksu.y = SortPoJednomIndeksu.x;
		SortPoJednomIndeksu.z = BROWN;
	}
	//orange
	SortPoJednomIndeksu.x = DuzinaFloatova3DBezKorenovanja(boja.x - 1, boja.y - 0.5, boja.z);
	if (SortPoJednomIndeksu.x < SortPoJednomIndeksu.y)
	{
		SortPoJednomIndeksu.y = SortPoJednomIndeksu.x;
		SortPoJednomIndeksu.z = ORANGE;
	}
	//gray
	SortPoJednomIndeksu.x = DuzinaFloatova3DBezKorenovanja(boja.x - 0.5, boja.y - 0.5, boja.z - 0.5);
	if (SortPoJednomIndeksu.x < SortPoJednomIndeksu.y)
	{
		SortPoJednomIndeksu.y = SortPoJednomIndeksu.x;
		SortPoJednomIndeksu.z = GREY;
	}
	//white
	SortPoJednomIndeksu.x = DuzinaFloatova3DBezKorenovanja(boja.x - 1, boja.y - 1, boja.z - 1);
	if (SortPoJednomIndeksu.x < SortPoJednomIndeksu.y)
	{
		SortPoJednomIndeksu.y = SortPoJednomIndeksu.x;
		SortPoJednomIndeksu.z = WHITE;
	}
	//black
	SortPoJednomIndeksu.x = DuzinaFloatova3DBezKorenovanja(boja.x, boja.y, boja.z);
	if (SortPoJednomIndeksu.x < SortPoJednomIndeksu.y)
	{
		SortPoJednomIndeksu.y = SortPoJednomIndeksu.x;
		SortPoJednomIndeksu.z = BLACK;
	}
	return SortPoJednomIndeksu.z;
}

struct Tacka2D DepthCalculator(struct Vektor3D Tacka)
{
	struct Tacka2D OutputScreenCoordinates;
	if (Tacka.z <= 0)
	{
		
		if (Tacka.x < 0) OutputScreenCoordinates.x = -WINDOW_WIDTH;
		else if (Tacka.x > 0)OutputScreenCoordinates.x = WINDOW_WIDTH;
		else OutputScreenCoordinates.x = 0;

		if (Tacka.y < 0) OutputScreenCoordinates.y = -WINDOW_HEIGHT;
		else if (Tacka.y > 0)OutputScreenCoordinates.y = WINDOW_HEIGHT;
		else OutputScreenCoordinates.y = 0;

		//OutputScreenCoordinates.x = Inrerpolate(0, Tacka.x, SkracenjeUOdnosuNaZ(Tacka.z, SKRACENJE));
		//OutputScreenCoordinates.y = Inrerpolate(0, Tacka.y, SkracenjeUOdnosuNaZ(Tacka.z, SKRACENJE));
	}
	else
	{
		OutputScreenCoordinates.x = SKRACENJE * Tacka.x / Tacka.z;
		OutputScreenCoordinates.y = SKRACENJE * Tacka.y / Tacka.z;
	}
	return OutputScreenCoordinates;
}
int backfaceCullQuad(const struct Vertex* VertsForBackfaceCheck, void* Polygon, int TriZeroOrQuadOne)
{

	struct Tacka2D VerteksiNaEkranu[3];//zadaje privremene vertekse za racunanje (3 su dovoljna
	float cosine, sine;

	if (TriZeroOrQuadOne)
	{
		VerteksiNaEkranu[0] = VertsForBackfaceCheck[((struct QuadPoly*)Polygon)->vertcies[0]].ScreenSpacePos;
		VerteksiNaEkranu[1] = VertsForBackfaceCheck[((struct QuadPoly*)Polygon)->vertcies[1]].ScreenSpacePos;
		VerteksiNaEkranu[2] = VertsForBackfaceCheck[((struct QuadPoly*)Polygon)->vertcies[2]].ScreenSpacePos;
	}
	else
	{

		VerteksiNaEkranu[0] = VertsForBackfaceCheck[((struct TriPoly*)Polygon)->vertcies[0]].ScreenSpacePos;
		VerteksiNaEkranu[1] = VertsForBackfaceCheck[((struct TriPoly*)Polygon)->vertcies[1]].ScreenSpacePos;
		VerteksiNaEkranu[2] = VertsForBackfaceCheck[((struct TriPoly*)Polygon)->vertcies[2]].ScreenSpacePos;
	}

	translate2D(3, &VerteksiNaEkranu[0].x, -VerteksiNaEkranu[0].x, -VerteksiNaEkranu[0].y);//Centar sistema je prvi verteks;

	//cosinus ugla dobijen skalarnim proizvodom izmedju x ose (tacke sa koordinatama (1,0)), prve tacke i druge tacke 
	cosine = VerteksiNaEkranu[1].x / DuzinaTacke2D(VerteksiNaEkranu[1]);
	//sinus tog ugla
	sine = sqrt(1 - cosine * cosine);
	if (VerteksiNaEkranu[1].y < 0) sine *= -1;

	//y pozicija trece tacke posle rotiranja za dobijeni ugao
	VerteksiNaEkranu[2].y = VerteksiNaEkranu[2].y * cosine - VerteksiNaEkranu[2].x * sine;



	//ako je y manje od 0 onda gleda ka kameri, a u suprotnom od kamere
	if (VerteksiNaEkranu[2].y < 0)
	{
		if (TriZeroOrQuadOne)
		{
			((struct QuadPoly*)Polygon)->UdaljenostOdKamere = 0;
			return 0;
		}
		else
		{
			((struct TriPoly*)Polygon)->UdaljenostOdKamere = 0;
			return 0;
		}

	}
	else
	{
		if (TriZeroOrQuadOne)
		{
			((struct QuadPoly*)Polygon)->UdaljenostOdKamere = 1;
			return 1;
		}
		else
		{
			((struct TriPoly*)Polygon)->UdaljenostOdKamere = 1;
			return 1;
		}

	}
}

void CalculatePolygonNormal(struct Vertex* ScreenSpaceVerts, struct QuadPoly* Polygon)
{

	struct Vektor3D a = { ScreenSpaceVerts[Polygon->vertcies[1]].pozicija.x - ScreenSpaceVerts[Polygon->vertcies[0]].pozicija.x, ScreenSpaceVerts[Polygon->vertcies[1]].pozicija.y - ScreenSpaceVerts[Polygon->vertcies[0]].pozicija.y, ScreenSpaceVerts[Polygon->vertcies[1]].pozicija.z - ScreenSpaceVerts[Polygon->vertcies[0]].pozicija.z };
	struct Vektor3D b = { ScreenSpaceVerts[Polygon->vertcies[1]].pozicija.x - ScreenSpaceVerts[Polygon->vertcies[2]].pozicija.x, ScreenSpaceVerts[Polygon->vertcies[1]].pozicija.y - ScreenSpaceVerts[Polygon->vertcies[2]].pozicija.y, ScreenSpaceVerts[Polygon->vertcies[1]].pozicija.z - ScreenSpaceVerts[Polygon->vertcies[2]].pozicija.z };
	//printf("\nVektor a = %f, y = %f, z = %f", a.x, a.y, a.z);
	//printf("\nVektor b = %f, y = %f, z = %f", b.x, b.y, b.z);

	Polygon->normala.x = a.y * b.z - a.z * b.y;
	Polygon->normala.y = a.z * b.x - a.x * b.z;
	Polygon->normala.z = a.x * b.y - a.y * b.z;
}

void RenderQuad(struct Vertex* ScreenSpaceVerts, struct QuadPoly* _Polygon, struct Scena* _Scena)
{
	float intensity = 0;
	struct Light* light;
	struct Vektor3D TempBoja = { 0,0,0 };
	struct Tacka3D SortPoJednomIndeksu;

	//calculateLighting
	CalculatePolygonNormal(ScreenSpaceVerts, _Polygon);

	for (int i = 0; i < _Scena->BrojSvetla; i++)
	{
		light = GetLightFromScene(_Scena, i);

		if (light->lightype == PointLight)
		{



		}

		if (light->lightype == DirectionalLight)
		{
			//skalarni proizvod pravca svetla i normale poligona
			intensity = (_Polygon->normala.x * light->Pozicija_Pravac.x + _Polygon->normala.y * light->Pozicija_Pravac.y + _Polygon->normala.z * light->Pozicija_Pravac.z) / (sqrt(_Polygon->normala.x * _Polygon->normala.x + _Polygon->normala.y * _Polygon->normala.y + _Polygon->normala.z * _Polygon->normala.z));


			if (intensity > 0)
			{
				TempBoja.x += intensity * light->intensity * light->boja.x;
				TempBoja.y += intensity * light->intensity * light->boja.y;
				TempBoja.z += intensity * light->intensity * light->boja.z;
			}
		}

		if (light->lightype == AmbientLight)
		{
			TempBoja.x += light->intensity * light->boja.x;
			TempBoja.y += light->intensity * light->boja.y;
			TempBoja.z += light->intensity * light->boja.z;
		}
	}


	TempBoja.x *= _Polygon->material->boja.x;
	TempBoja.y *= _Polygon->material->boja.y;
	TempBoja.z *= _Polygon->material->boja.z;



	fill(ProznadjiBoju(TempBoja));
	stroke(BLACK);
	strokeweight(1);
	quadrangle(ScreenSpaceVerts[_Polygon->vertcies[0]].ScreenSpacePos.x, ScreenSpaceVerts[_Polygon->vertcies[0]].ScreenSpacePos.y, ScreenSpaceVerts[_Polygon->vertcies[1]].ScreenSpacePos.x, ScreenSpaceVerts[_Polygon->vertcies[1]].ScreenSpacePos.y,
		ScreenSpaceVerts[_Polygon->vertcies[2]].ScreenSpacePos.x, ScreenSpaceVerts[_Polygon->vertcies[2]].ScreenSpacePos.y, ScreenSpaceVerts[_Polygon->vertcies[3]].ScreenSpacePos.x, ScreenSpaceVerts[_Polygon->vertcies[3]].ScreenSpacePos.y);
}
void RenderTri(struct Vertex* ScreenSpaceVerts, struct TriPoly* Polygon, struct Scena* _Scena)
{
	stroke(BLACK);
	strokeweight(1);
	triangle(ScreenSpaceVerts[Polygon->vertcies[0]].ScreenSpacePos.x, ScreenSpaceVerts[Polygon->vertcies[0]].ScreenSpacePos.y, ScreenSpaceVerts[Polygon->vertcies[1]].ScreenSpacePos.x, ScreenSpaceVerts[Polygon->vertcies[1]].ScreenSpacePos.y,
		ScreenSpaceVerts[Polygon->vertcies[2]].ScreenSpacePos.x, ScreenSpaceVerts[Polygon->vertcies[2]].ScreenSpacePos.y);
}
void RenderLine(struct Vertex* ScreenSpaceVerts, struct Line* Polygon)
{
	stroke(Polygon->boja);
	strokeweight(2);
	line(ScreenSpaceVerts[Polygon->vertcies[0]].ScreenSpacePos.x, ScreenSpaceVerts[Polygon->vertcies[0]].ScreenSpacePos.y, ScreenSpaceVerts[Polygon->vertcies[1]].ScreenSpacePos.x, ScreenSpaceVerts[Polygon->vertcies[1]].ScreenSpacePos.y);
}


/////////////////////////////////////////////renderovanje modela i instanci modela/////////////////////////////////////////////////////////////////////


void RenderModel(struct Model* _Model, struct TwoIntsAndFloat* Sort, struct Scena* _Scena, struct Vertex* TempVerts)
{
	int j = 0;
	struct Vektor3D Pivot;

	Pivot = _Model->Pivot;
	Pivot.x -= _Scena->Kamera.Pozicija.x;
	Pivot.y -= _Scena->Kamera.Pozicija.y;
	Pivot.z -= _Scena->Kamera.Pozicija.z;
	RotateVectorAroundY3DByVector(&Pivot, _Scena->Kamera.Pravac);


	for (int i = 0; i < _Model->NumOfVerts; i++)
	{
		TempVerts[i] = _Model->Vertexptr[i];
		//transformacije kamere
		TempVerts[i].pozicija.x -= _Scena->Kamera.Pozicija.x;
		TempVerts[i].pozicija.y -= _Scena->Kamera.Pozicija.y;
		TempVerts[i].pozicija.z -= _Scena->Kamera.Pozicija.z;
		RotateVectorAroundY3DByVector(&TempVerts[i].pozicija, _Scena->Kamera.Pravac);

		TempVerts[i].ScreenSpacePos = DepthCalculator(TempVerts[i].pozicija);
	}

	for (int i = 0; i < _Model->NumOfQuads; i++)
	{
		if (!backfaceCullQuad(TempVerts, &_Model->QuadPtr[i], 1))//backface cull
		{
			Sort[j].x = i;
			Sort[j].y = 0;
			Sort[j].z = DuzinaVektora3DBezKorenovanja(SrednjiVerteksQuada(TempVerts, &_Model->QuadPtr[i]));
			j++;
		}
	}
	for (int i = 0; i < _Model->NumOfTris; i++)
	{
		if (!backfaceCullQuad(TempVerts, &_Model->QuadPtr[i], 1))//backface cull stavlja udaljenost od kamere na 0 ako je cullovan, a 1 ako nije
		{
			Sort[j].x = i;
			Sort[j].y = 1;
			Sort[j].z = DuzinaVektora3DBezKorenovanja(SrednjiVerteksTria(TempVerts, &_Model->TriPtr[i]));
			j++;
		}
	}

	for (int i = 0; i < _Model->NumOfLines; i++)//linije nemaju backface lol
	{
		Sort[j].x = i;
		Sort[j].y = 2;
		Sort[j].z = DuzinaVektora3DBezKorenovanja(SrednjiVerteksLinije(TempVerts, &_Model->LinePtr[i]));
		j++;
	}

	quickSort(Sort, 0, j - 1);//sortira elemente za crtanje po udaljenosti od kamere

	for (int i = 0; i < j; i++)//crta sve elemente modela
	{
		if (Sort[i].y == 0) RenderQuad(TempVerts, &_Model->QuadPtr[Sort[i].x], _Scena);
		if (Sort[i].y == 1) RenderTri(TempVerts, &_Model->TriPtr[Sort[i].x], _Scena);
		if (Sort[i].y == 2) RenderLine(TempVerts, &_Model->LinePtr[Sort[i].x]);
	}
	if (SHOW_PIVOT)
	{
		fill(RED);
		ellipse(DepthCalculator(Pivot).x, DepthCalculator(Pivot).y, 5, 5);
	}
}
//////////////////////////////////////////////////render model instance////////////////////////////////////////////////////////
void RenderModelInstance(struct ModelInstance* _ModelInst, struct TwoIntsAndFloat* Sort, struct Scena* _Scena, struct Vertex* TempVerts)
{
	struct Tacka3D TempVerteks;
	int j = 0;
	struct Vektor3D Pivot;

	Pivot = _ModelInst->Translate;
	Pivot.x -= _Scena->Kamera.Pozicija.x;
	Pivot.y -= _Scena->Kamera.Pozicija.y;
	Pivot.z -= _Scena->Kamera.Pozicija.z;
	RotateVectorAroundY3DByVector(&Pivot, _Scena->Kamera.Pravac);

	/*********************************izvrsaavanje transformacija instanci na verteksu*********************************/

	for (int i = 0; i < _ModelInst->ModelReference->NumOfVerts; i++)
	{
		TempVerts[i].pozicija = _ModelInst->ModelReference->Vertexptr[i].pozicija;

		TempVerts[i].pozicija.x -= _ModelInst->ModelReference->Pivot.x;//vraca koordinate na 0,0 za scale i rotate
		TempVerts[i].pozicija.y -= _ModelInst->ModelReference->Pivot.y;
		TempVerts[i].pozicija.z -= _ModelInst->ModelReference->Pivot.z;

		if (_ModelInst->scale - 1)//ako je scale pomeren
		{
			TempVerts[i].pozicija.x *= _ModelInst->scale;
			TempVerts[i].pozicija.y *= _ModelInst->scale;
			TempVerts[i].pozicija.z *= _ModelInst->scale;
		}
		if (_ModelInst->Rotate.x)
		{

		}
		if (_ModelInst->Rotate.y)
		{
			TempVerteks.x = TempVerts[i].pozicija.x;
			TempVerteks.z = TempVerts[i].pozicija.z;
			TempVerts[i].pozicija.x = TempVerteks.x*cos(_ModelInst->Rotate.y) - TempVerteks.z * sin(_ModelInst->Rotate.y);
			TempVerts[i].pozicija.z = TempVerteks.x*sin(_ModelInst->Rotate.y) + TempVerteks.z * cos(_ModelInst->Rotate.y);
		}
		if (_ModelInst->Rotate.z)
		{

		}

		//printf("\nkoordinate tacaka pre pomeraja = %d,%d,%d", _ModelInst->Translate.x, _ModelInst->Translate.y, _ModelInst->Translate.z);
		TempVerts[i].pozicija.x += _ModelInst->Translate.x;//izvrsava translaciju instance u odnosu na originalan model
		TempVerts[i].pozicija.y += _ModelInst->Translate.y;
		TempVerts[i].pozicija.z += _ModelInst->Translate.z;

		//camera transformations

		TempVerts[i].pozicija.x -= _Scena->Kamera.Pozicija.x;//izvrsava translaciju instance u odnosu na originalan model
		TempVerts[i].pozicija.y -= _Scena->Kamera.Pozicija.y;
		TempVerts[i].pozicija.z -= _Scena->Kamera.Pozicija.z;
		RotateVectorAroundY3DByVector(&TempVerts[i].pozicija, _Scena->Kamera.Pravac);


		TempVerts[i].ScreenSpacePos = DepthCalculator(TempVerts[i].pozicija);//racuna screenspace pozicije verteksa

	}

	/*********************************sortiranje elemenata*********************************/
	for (int i = 0; i < _ModelInst->ModelReference->NumOfQuads; i++)
	{
		if (!backfaceCullQuad(TempVerts, &_ModelInst->ModelReference->QuadPtr[i], 1))
		{
			Sort[j].x = i;
			Sort[j].y = 0;
			Sort[j].z = DuzinaVektora3DBezKorenovanja(SrednjiVerteksQuada(TempVerts, &_ModelInst->ModelReference->QuadPtr[i]));
			j++;
		}

	}
	for (int i = 0; i < _ModelInst->ModelReference->NumOfTris; i++)
	{
		if (!backfaceCullQuad(TempVerts, &_ModelInst->ModelReference->TriPtr[i], 0))
		{
			Sort[j].x = i;
			Sort[j].y = 1;
			Sort[j].z = DuzinaVektora3DBezKorenovanja(SrednjiVerteksTria(TempVerts, &_ModelInst->ModelReference->TriPtr[i]));
			j++;
		}
	}

	for (int i = 0; i < _ModelInst->ModelReference->NumOfLines; i++)//linije nemaju backface lol
	{
		Sort[i].x = i;
		Sort[i].y = 2;
		Sort[i].z = DuzinaVektora3DBezKorenovanja(SrednjiVerteksLinije(TempVerts, &_ModelInst->ModelReference->LinePtr[i]));
		j++;
	}

	quickSort(Sort, 0, j - 1);//sortira elemente za crtanje po udaljenosti od kamere

	for (int i = 0; i < j; i++)//crta sve elemente modela
	{
		if (Sort[i].y == 0) RenderQuad(TempVerts, &_ModelInst->ModelReference->QuadPtr[Sort[i].x], _Scena);
		if (Sort[i].y == 1) RenderTri(TempVerts, &_ModelInst->ModelReference->TriPtr[Sort[i].x], _Scena);
		if (Sort[i].y == 2) RenderLine(TempVerts, &_ModelInst->ModelReference->LinePtr[Sort[i].x]);
	}

	if (SHOW_PIVOT)
	{
		fill(RED);
		ellipse(DepthCalculator(Pivot).x, DepthCalculator(Pivot).y, 5, 5);
	}
}


/////////////////////////////////////////////////draw scene//////////////////////////////////////////////////


void DrawScene(struct Scena* _Scena, int frameNumber)
{
	struct Vektor3D Pivot;

	struct TwoIntsAndFloat* SortModela = (struct TwoIntsAndFloat*)calloc(_Scena->BrojInstancimodela + _Scena->BrojModela, sizeof(struct TwoIntsAndFloat));
	struct TwoIntsAndFloat* SortElemenata = (struct TwoIntsAndFloat*)calloc(_Scena->BrojStvariZaSortiranje, sizeof(struct TwoIntsAndFloat));
	struct Vertex* TempVerts = (struct Vertex*)calloc(_Scena->BrojTempVerteksa, sizeof(struct Vertex));
	struct Vektor3D* WorldPosOfLights = NULL;
	if (_Scena->BrojSvetla) WorldPosOfLights = (struct Vektor3D*)calloc(_Scena->BrojSvetla, sizeof(struct Vektor3D));
	int j = 0;
	
	struct Model* TempModel;
	struct ModelInstance* TempInstance;
	struct Light* TempSvetlo;

	for (int i = 0; i < _Scena->BrojSvetla; i++)
	{
		TempSvetlo = GetLightFromScene(_Scena, i);
		WorldPosOfLights[i] = TempSvetlo->Pozicija_Pravac;
		if (TempSvetlo->lightype == DirectionalLight)
		{
			int pozicijaSuncaX, pozicijaSuncaY;
			RotateVectorAroundY3DByVector(&TempSvetlo->Pozicija_Pravac, _Scena->Kamera.Pravac);//namesta svetlo u odnosu na rotaciju kamere
			NormalizeVektor3D(&TempSvetlo->Pozicija_Pravac);

			//draw directional light as the sun

			if (TempSvetlo->Pozicija_Pravac.z < -0.1)//da li je sunce na ekranu
			{
				//zrak sunca
				struct Tacka2D Zrak[3];
				Zrak[0].x = 0;
				Zrak[0].y = 3;
				Zrak[1].x = 60;
				Zrak[1].y = 0;
				Zrak[2].x = 0;
				Zrak[2].y = -3;


				pozicijaSuncaX = TempSvetlo->Pozicija_Pravac.x * SKRACENJE / TempSvetlo->Pozicija_Pravac.z;
				pozicijaSuncaY = TempSvetlo->Pozicija_Pravac.y * SKRACENJE / TempSvetlo->Pozicija_Pravac.z;

				fill(YELLOW);
				ellipse(pozicijaSuncaX, pozicijaSuncaY, 25, 25);


				translate2D(3, &Zrak[0].x, pozicijaSuncaX + 50, pozicijaSuncaY);//pomera zrak sa 0,0 na prvu poziciju
				Rotate2D(3, &Zrak[0].x, NapraviTacku2D(pozicijaSuncaX, pozicijaSuncaY), DegreeToRadians(frameNumber * 10));//rotira oko centra sunca

				for (int i = 0; i < 8; i++)
				{
					triangle(Zrak[0].x, Zrak[0].y, Zrak[1].x, Zrak[1].y, Zrak[2].x, Zrak[2].y);
					Rotate2D(3, &Zrak[0].x, NapraviTacku2D(pozicijaSuncaX, pozicijaSuncaY), DegreeToRadians(360.0 / 8));//rotira oko centra sunca
				}
			}
		}
	}

	for (int i = 0; i < _Scena->BrojModela; i++)//pokazuje prve pointere liste ka ScreenSpace verteksima modela scene
	{
		TempModel = GetModelFromScene(_Scena, i);
		Pivot = TempModel->Pivot;
		Pivot.x -= _Scena->Kamera.Pozicija.x;
		Pivot.y -= _Scena->Kamera.Pozicija.y;
		Pivot.z -= _Scena->Kamera.Pozicija.z;
		RotateVectorAroundY3DByVector(&Pivot, _Scena->Kamera.Pravac);


		if (Pivot.z > -TempModel->BoundingRadious)
		{
			SortModela[j].x = i;//priprema listu za sortiranje po udaljenosti
			SortModela[j].y = 0;
			printf("\nPivot.z = %f", Pivot.z);
			SortModela[j].z = DuzinaVektora3DBezKorenovanja(Pivot);
			j++;
		}
	}
	for (int i = 0; i < _Scena->BrojInstancimodela; i++)
	{
		TempInstance = GetModelInstaneFromScene(_Scena, i);

		Pivot = TempInstance->Translate;
		Pivot.x -= _Scena->Kamera.Pozicija.x;
		Pivot.y -= _Scena->Kamera.Pozicija.y;
		Pivot.z -= _Scena->Kamera.Pozicija.z;
		RotateVectorAroundY3DByVector(&Pivot, _Scena->Kamera.Pravac);
		
		if (Pivot.z > -TempInstance->ModelReference->BoundingRadious * TempInstance->scale)
		{
			SortModela[j].x = i;//priprema listu za sortiranje po udaljenosti
			SortModela[j].y = 1;
			printf("\nPivot.z = %f", Pivot.z);
			SortModela[j].z = DuzinaVektora3DBezKorenovanja(Pivot);
			j++;
		}
	}


	//sortiranje modela po udaljenosti njihovih pivota od kamere(0,0)


	quickSort(SortModela, 0, j-1);


	for (int i = 0; i < j; i++)
	{
		if (SortModela[i].y == 0)//index 2 razlikuje model od instance
		{
			RenderModel(GetModelFromScene(_Scena, SortModela[i].x), SortElemenata, _Scena, TempVerts);
		}
		else
		{
			RenderModelInstance(GetModelInstaneFromScene(_Scena, SortModela[i].x), SortElemenata, _Scena, TempVerts);
		}
	}

	if (_Scena->BrojSvetla)//vraca svetla na worldspace
	{
		for (int i = 0; i < _Scena->BrojSvetla; i++)
		{
			TempSvetlo = GetLightFromScene(_Scena, i);//vraca rotaciju ili polozaj svetla sa camera space na world space
			TempSvetlo->Pozicija_Pravac = WorldPosOfLights[i];
		}
		free(WorldPosOfLights);
	}


	free(SortModela);
	free(SortElemenata);
	free(TempVerts);
}


//////////////////////////////////////"modelovanje"///////////////////////////////////////

void MoveVerts(struct Vertex* vertices, unsigned int PocetniVerteks, unsigned int BrojVerteksZaPomeranje, struct Vektor3D Pomeraj)
{
	for (int i = 0; i < BrojVerteksZaPomeranje; i++)
	{
		vertices[PocetniVerteks + i].pozicija.x += Pomeraj.x;
		vertices[PocetniVerteks + i].pozicija.y += Pomeraj.y;
		vertices[PocetniVerteks + i].pozicija.z += Pomeraj.z;
	}
}

void MakeCube(struct Vertex* vertices, unsigned int PocetniVerteks, struct QuadPoly* QuadPolygons, unsigned int PocetniQuadPoly, int x, int y, int z, struct Material* _Materijal)
{
	x /= 2;
	y /= 2;
	z /= 2;



	vertices[0 + PocetniVerteks].pozicija.x = -x;
	vertices[0 + PocetniVerteks].pozicija.y = y;
	vertices[0 + PocetniVerteks].pozicija.z = z;

	vertices[1 + PocetniVerteks].pozicija.x = x;
	vertices[1 + PocetniVerteks].pozicija.y = y;
	vertices[1 + PocetniVerteks].pozicija.z = z;

	vertices[2 + PocetniVerteks].pozicija.x = x;
	vertices[2 + PocetniVerteks].pozicija.y = -y;
	vertices[2 + PocetniVerteks].pozicija.z = z;

	vertices[3 + PocetniVerteks].pozicija.x = -x;
	vertices[3 + PocetniVerteks].pozicija.y = -y;
	vertices[3 + PocetniVerteks].pozicija.z = z;

	vertices[4 + PocetniVerteks].pozicija.x = -x;
	vertices[4 + PocetniVerteks].pozicija.y = y;
	vertices[4 + PocetniVerteks].pozicija.z = -z;

	vertices[5 + PocetniVerteks].pozicija.x = x;
	vertices[5 + PocetniVerteks].pozicija.y = y;
	vertices[5 + PocetniVerteks].pozicija.z = -z;

	vertices[6 + PocetniVerteks].pozicija.x = x;
	vertices[6 + PocetniVerteks].pozicija.y = -y;
	vertices[6 + PocetniVerteks].pozicija.z = -z;

	vertices[7 + PocetniVerteks].pozicija.x = -x;
	vertices[7 + PocetniVerteks].pozicija.y = -y;
	vertices[7 + PocetniVerteks].pozicija.z = -z;

	QuadPolygons[0 + PocetniQuadPoly].vertcies[0] = 1 + PocetniVerteks;
	QuadPolygons[0 + PocetniQuadPoly].vertcies[1] = 0 + PocetniVerteks;
	QuadPolygons[0 + PocetniQuadPoly].vertcies[2] = 3 + PocetniVerteks;
	QuadPolygons[0 + PocetniQuadPoly].vertcies[3] = 2 + PocetniVerteks;
	QuadPolygons[0 + PocetniQuadPoly].material = _Materijal;

	QuadPolygons[1 + PocetniQuadPoly].vertcies[0] = 5 + PocetniVerteks;
	QuadPolygons[1 + PocetniQuadPoly].vertcies[1] = 1 + PocetniVerteks;
	QuadPolygons[1 + PocetniQuadPoly].vertcies[2] = 2 + PocetniVerteks;
	QuadPolygons[1 + PocetniQuadPoly].vertcies[3] = 6 + PocetniVerteks;
	QuadPolygons[1 + PocetniQuadPoly].material = _Materijal;

	QuadPolygons[2 + PocetniQuadPoly].vertcies[0] = 0 + PocetniVerteks;
	QuadPolygons[2 + PocetniQuadPoly].vertcies[1] = 4 + PocetniVerteks;
	QuadPolygons[2 + PocetniQuadPoly].vertcies[2] = 7 + PocetniVerteks;
	QuadPolygons[2 + PocetniQuadPoly].vertcies[3] = 3 + PocetniVerteks;
	QuadPolygons[2 + PocetniQuadPoly].material = _Materijal;

	QuadPolygons[3 + PocetniQuadPoly].vertcies[0] = 0 + PocetniVerteks;
	QuadPolygons[3 + PocetniQuadPoly].vertcies[1] = 1 + PocetniVerteks;
	QuadPolygons[3 + PocetniQuadPoly].vertcies[2] = 5 + PocetniVerteks;
	QuadPolygons[3 + PocetniQuadPoly].vertcies[3] = 4 + PocetniVerteks;
	QuadPolygons[3 + PocetniQuadPoly].material = _Materijal;

	QuadPolygons[4 + PocetniQuadPoly].vertcies[0] = 2 + PocetniVerteks;
	QuadPolygons[4 + PocetniQuadPoly].vertcies[1] = 3 + PocetniVerteks;
	QuadPolygons[4 + PocetniQuadPoly].vertcies[2] = 7 + PocetniVerteks;
	QuadPolygons[4 + PocetniQuadPoly].vertcies[3] = 6 + PocetniVerteks;
	QuadPolygons[4 + PocetniQuadPoly].material = _Materijal;

	QuadPolygons[5 + PocetniQuadPoly].vertcies[0] = 4 + PocetniVerteks;
	QuadPolygons[5 + PocetniQuadPoly].vertcies[1] = 5 + PocetniVerteks;
	QuadPolygons[5 + PocetniQuadPoly].vertcies[2] = 6 + PocetniVerteks;
	QuadPolygons[5 + PocetniQuadPoly].vertcies[3] = 7 + PocetniVerteks;
	QuadPolygons[5 + PocetniQuadPoly].material = _Materijal;
}


int main() {

	///////////////////////////////////////////podatci///////////////////////////////////////////
	int polozajKosiliceX;
	int polozajKosiliceZ;
	int UgaoKosilice = 0;
	int BrzinaKosilice = 120;
	int GraniceKosiliceX, GraniceKosiliceZ;
	struct Vektor3D TempPivot;

	int duzinaTravnjaka = 0;
	int SirinaTravnjaka = 0;
	int SirinaOgrade = 0;
	int duzinaOgrade = 6;
	int RazmakIzmedjuStubova = 250;
	int visinaStuba = 200;
	int MaksSirinaOgrade = 2500;

	int brojZrakaSunca = 8;
	int pozicijaSuncaX = 200;
	int	pozicijaSuncaY = 120;

	int exit = 0;
	char key;
	int frameNumber = 0;



	///////////////////////////////////////////unos podataka///////////////////////////////////////////
	printf("Molim vas unesite duzinu sirinu travnjaka \n(raste po inkrementima od %d, minimum 0, maksimum: %d) :", RazmakIzmedjuStubova, MaksSirinaOgrade);
	scanf("%d", &SirinaTravnjaka);

	if (SirinaTravnjaka < 0) SirinaTravnjaka = 0;
	if (SirinaTravnjaka > MaksSirinaOgrade) SirinaTravnjaka = MaksSirinaOgrade;
	SirinaOgrade = (SirinaTravnjaka / RazmakIzmedjuStubova) + 1;

	SirinaTravnjaka = SirinaOgrade * RazmakIzmedjuStubova;
	duzinaTravnjaka = duzinaOgrade * RazmakIzmedjuStubova;
	printf("\nSirina = %d, Duzina = %d", SirinaTravnjaka, duzinaTravnjaka);


	printf("\nMolim vas unesite ugao kosilice :");
	scanf("%d", &UgaoKosilice);
	while (UgaoKosilice > 360) UgaoKosilice -= 360;
	while (UgaoKosilice < 0) UgaoKosilice += 360;

	printf("\nMolim vas unesite polozaj kosilice \n(min x %d, max x %d, min z %d, max z %d) :", -((SirinaOgrade)* RazmakIzmedjuStubova) / 2 + 125, ((SirinaOgrade)* RazmakIzmedjuStubova) / 2 - 125, 0, RazmakIzmedjuStubova * duzinaOgrade - 60);
	scanf("%d %d", &polozajKosiliceX, &polozajKosiliceZ);

	//da li je kosilica u granicama ograde
	if (polozajKosiliceX < -SirinaTravnjaka / 2 + 125) polozajKosiliceX = -SirinaTravnjaka / 2 + 125;
	if (polozajKosiliceX > SirinaTravnjaka / 2 - 125) polozajKosiliceX = SirinaTravnjaka / 2 - 125;


	if (polozajKosiliceZ < 125) polozajKosiliceZ = 125;
	if (polozajKosiliceZ > duzinaTravnjaka - 125) polozajKosiliceZ = duzinaTravnjaka - 125;

	//////////////////////////////////////////////////definianje scene////////////////////////////////////////

	struct Scena BozePomozi_Scena = INITIALIZE_SCENE;

	struct Light sunce = { .intensity = 3,.lightype = DirectionalLight,.Pozicija_Pravac = {.x = -150,.y = -100,.z = -280},.boja = {1,1,1} };
	struct Light nebo = { .intensity = 0.8,.lightype = AmbientLight,.boja = {0.9,0.8,1} };

	AddLightToScene(&BozePomozi_Scena, &sunce);
	AddLightToScene(&BozePomozi_Scena, &nebo);

	/////////////////////////////////////////////////////////inicijalizacija modela///////////////////////////////////////////////////////////





	/********************************Kosilica*************************************/
	//materijali
	struct Material CrvenaPlastika = { .roughness = 0.3,.metalic = 0,.boja = {1,0,0} };
	struct Material SiviMetal = { .roughness = 0.2,.metalic = 1,.boja = {0.5,0.5,0.5} };






	struct Model Kosilica = INITIALIZE_MODEL;
	Kosilica.naziv[0] = 'k';
	Kosilica.naziv[1] = 'o';
	Kosilica.naziv[2] = 's';

	Kosilica.BoundingRadious = 125;

	struct Vertex VerteksiKosilice[18];
	Kosilica.NumOfVerts = 18;
	Kosilica.Vertexptr = VerteksiKosilice;

	struct QuadPoly PoligoniKosilice[14];
	Kosilica.NumOfQuads = 14;
	Kosilica.QuadPtr = PoligoniKosilice;

	struct Line LinijeKosilice[3];
	Kosilica.NumOfLines = 3;
	Kosilica.LinePtr = LinijeKosilice;

	Kosilica.Pivot = NapraviVektor3D(0, 0, 0);

	Kosilica.ImaPravac = 1;
	Kosilica.Pravac = NapraviVektor3D(1, 0, 0);

	MakeCube(VerteksiKosilice, 0, PoligoniKosilice, 0, 200, 30, 100, &CrvenaPlastika);//telo
	MakeCube(VerteksiKosilice, 8, PoligoniKosilice, 6, 90, 20, 90, &SiviMetal);//motor

	PoligoniKosilice[3].vertcies[0] = 4;
	PoligoniKosilice[3].vertcies[1] = 15;
	PoligoniKosilice[3].vertcies[2] = 14;
	PoligoniKosilice[3].vertcies[3] = 5;
	PoligoniKosilice[3].material = &CrvenaPlastika;

	PoligoniKosilice[10].vertcies[0] = 0;
	PoligoniKosilice[10].vertcies[1] = 11;
	PoligoniKosilice[10].vertcies[2] = 15;
	PoligoniKosilice[10].vertcies[3] = 4;
	PoligoniKosilice[10].material = &CrvenaPlastika;

	PoligoniKosilice[12].vertcies[0] = 0;
	PoligoniKosilice[12].vertcies[1] = 1;
	PoligoniKosilice[12].vertcies[2] = 10;
	PoligoniKosilice[12].vertcies[3] = 11;
	PoligoniKosilice[12].material = &CrvenaPlastika;

	PoligoniKosilice[13].vertcies[0] = 5;
	PoligoniKosilice[13].vertcies[1] = 14;
	PoligoniKosilice[13].vertcies[2] = 10;
	PoligoniKosilice[13].vertcies[3] = 1;
	PoligoniKosilice[13].material = &CrvenaPlastika;

	MoveVerts(VerteksiKosilice, 8, 8, NapraviVektor3D(-20, 25, 0));//pomera motor

	//drske
	VerteksiKosilice[16].pozicija.x = -180;
	VerteksiKosilice[16].pozicija.y = 100;
	VerteksiKosilice[16].pozicija.z = +45;

	VerteksiKosilice[17].pozicija.x = -180;
	VerteksiKosilice[17].pozicija.y = 100;
	VerteksiKosilice[17].pozicija.z = -45;


	LinijeKosilice[0].vertcies[0] = 8;
	LinijeKosilice[0].vertcies[1] = 16;
	LinijeKosilice[0].boja = BLACK;
	LinijeKosilice[0].debljina = 5;

	LinijeKosilice[1].vertcies[0] = 16;
	LinijeKosilice[1].vertcies[1] = 17;
	LinijeKosilice[1].boja = BLACK;
	LinijeKosilice[1].debljina = 5;

	LinijeKosilice[2].vertcies[0] = 17;
	LinijeKosilice[2].vertcies[1] = 12;
	LinijeKosilice[2].boja = BLACK;
	LinijeKosilice[2].debljina = 5;


	//tockovi

	/*kosilica[18].x = polozajKosiliceX - 75;
	kosilica[18].y = 6;
	kosilica[18].z = polozajKosiliceZ - 45;

	kosilica[19].x = polozajKosiliceX + 75;
	kosilica[19].y = 6;
	kosilica[19].z = polozajKosiliceZ - 45;*/

	TranslateModel3D(&Kosilica, NapraviVektor3D(polozajKosiliceX, 0, polozajKosiliceZ + 50));
	RotateAroundY3D(&Kosilica, DegreeToRadians(UgaoKosilice + 90));

	AddModelToScene(&BozePomozi_Scena, &Kosilica);

	/******************************Daske ograde***************************************/
	struct Material Drvo = { .roughness = 0.8,.metalic = 0,.boja = {0.6,0.2,0} };


	struct Model DaskeOgrade = INITIALIZE_MODEL;
	DaskeOgrade.naziv[0] = 'd';
	DaskeOgrade.naziv[1] = 'a';
	DaskeOgrade.naziv[2] = 's';

	DaskeOgrade.BoundingRadious = 60;

	struct Vertex VerteksiDaskaOgrade[16];
	DaskeOgrade.NumOfVerts = 16;
	DaskeOgrade.Vertexptr = VerteksiDaskaOgrade;

	struct QuadPoly PoligoniDaskaOgrade[12];
	DaskeOgrade.NumOfQuads = 12;
	DaskeOgrade.QuadPtr = PoligoniDaskaOgrade;

	DaskeOgrade.Pivot = NapraviVektor3D(0, 100, 0);

	MakeCube(VerteksiDaskaOgrade, 0, PoligoniDaskaOgrade, 0, 12, 40, RazmakIzmedjuStubova - 16, &Drvo);
	MoveVerts(VerteksiDaskaOgrade, 0, 8, NapraviVektor3D(0, 150, 0));

	MakeCube(VerteksiDaskaOgrade, 8, PoligoniDaskaOgrade, 6, 12, 40, RazmakIzmedjuStubova - 16, &Drvo);
	MoveVerts(VerteksiDaskaOgrade, 8, 8, NapraviVektor3D(0, 50, 0));

	TranslateModel3D(&DaskeOgrade, NapraviVektor3D(-SirinaOgrade * RazmakIzmedjuStubova / 2, 0, RazmakIzmedjuStubova / 2));

	AddModelToScene(&BozePomozi_Scena, &DaskeOgrade);


	/************************Stub Ograde*******************************/


	struct Model Ograda = INITIALIZE_MODEL;
	//InitializeModelIntoScene(&BozePomozi_Scena, &Ograda);

	DaskeOgrade.naziv[0] = 'o';
	DaskeOgrade.naziv[1] = 'g';
	DaskeOgrade.naziv[2] = 'r';

	DaskeOgrade.BoundingRadious = (visinaStuba + 9) / 2;

	struct Vertex VerteksiOgrade[12]; //kordinate stuba
	Ograda.NumOfVerts = 12;
	Ograda.Vertexptr = VerteksiOgrade;

	struct QuadPoly PoligoniOgrade[10];
	Ograda.NumOfQuads = 10;
	Ograda.QuadPtr = PoligoniOgrade;

	Ograda.Pivot = NapraviVektor3D(0, 100, 0);

	VerteksiOgrade[0].pozicija.x = -14;
	VerteksiOgrade[0].pozicija.y = 0;
	VerteksiOgrade[0].pozicija.z = 14;

	VerteksiOgrade[1].pozicija.x = 14;
	VerteksiOgrade[1].pozicija.y = 0;
	VerteksiOgrade[1].pozicija.z = 14;

	VerteksiOgrade[2].pozicija.x = 14;
	VerteksiOgrade[2].pozicija.y = 0;
	VerteksiOgrade[2].pozicija.z = -14;

	VerteksiOgrade[3].pozicija.x = -14;
	VerteksiOgrade[3].pozicija.y = 0;
	VerteksiOgrade[3].pozicija.z = -14;

	VerteksiOgrade[4].pozicija.x = -14;
	VerteksiOgrade[4].pozicija.y = visinaStuba;
	VerteksiOgrade[4].pozicija.z = 14;

	VerteksiOgrade[5].pozicija.x = 14;
	VerteksiOgrade[5].pozicija.y = visinaStuba;
	VerteksiOgrade[5].pozicija.z = 14;

	VerteksiOgrade[6].pozicija.x = 14;
	VerteksiOgrade[6].pozicija.y = visinaStuba;
	VerteksiOgrade[6].pozicija.z = -14;

	VerteksiOgrade[7].pozicija.x = -14;
	VerteksiOgrade[7].pozicija.y = visinaStuba;
	VerteksiOgrade[7].pozicija.z = -14;

	VerteksiOgrade[8].pozicija.x = -8;
	VerteksiOgrade[8].pozicija.y = visinaStuba + 7;
	VerteksiOgrade[8].pozicija.z = 8;

	VerteksiOgrade[9].pozicija.x = 8;
	VerteksiOgrade[9].pozicija.y = visinaStuba + 7;
	VerteksiOgrade[9].pozicija.z = 8;

	VerteksiOgrade[10].pozicija.x = 8;
	VerteksiOgrade[10].pozicija.y = visinaStuba + 7;
	VerteksiOgrade[10].pozicija.z = -8;

	VerteksiOgrade[11].pozicija.x = -8;
	VerteksiOgrade[11].pozicija.y = visinaStuba + 7;
	VerteksiOgrade[11].pozicija.z = -8;


	PoligoniOgrade[0].vertcies[0] = 8;
	PoligoniOgrade[0].vertcies[1] = 9;
	PoligoniOgrade[0].vertcies[2] = 10;
	PoligoniOgrade[0].vertcies[3] = 11;
	PoligoniOgrade[0].material = &Drvo;

	PoligoniOgrade[1].vertcies[0] = 9;
	PoligoniOgrade[1].vertcies[1] = 8;
	PoligoniOgrade[1].vertcies[2] = 4;
	PoligoniOgrade[1].vertcies[3] = 5;
	PoligoniOgrade[1].material = &Drvo;

	PoligoniOgrade[2].vertcies[0] = 8;
	PoligoniOgrade[2].vertcies[1] = 11;
	PoligoniOgrade[2].vertcies[2] = 7;
	PoligoniOgrade[2].vertcies[3] = 4;
	PoligoniOgrade[2].material = &Drvo;

	PoligoniOgrade[3].vertcies[0] = 10;
	PoligoniOgrade[3].vertcies[1] = 9;
	PoligoniOgrade[3].vertcies[2] = 5;
	PoligoniOgrade[3].vertcies[3] = 6;
	PoligoniOgrade[3].material = &Drvo;

	PoligoniOgrade[4].vertcies[0] = 11;
	PoligoniOgrade[4].vertcies[1] = 10;
	PoligoniOgrade[4].vertcies[2] = 6;
	PoligoniOgrade[4].vertcies[3] = 7;
	PoligoniOgrade[4].material = &Drvo;

	PoligoniOgrade[5].vertcies[0] = 5;
	PoligoniOgrade[5].vertcies[1] = 4;
	PoligoniOgrade[5].vertcies[2] = 0;
	PoligoniOgrade[5].vertcies[3] = 1;
	PoligoniOgrade[5].material = &Drvo;

	PoligoniOgrade[6].vertcies[0] = 4;
	PoligoniOgrade[6].vertcies[1] = 7;
	PoligoniOgrade[6].vertcies[2] = 3;
	PoligoniOgrade[6].vertcies[3] = 0;
	PoligoniOgrade[6].material = &Drvo;

	PoligoniOgrade[7].vertcies[0] = 6;
	PoligoniOgrade[7].vertcies[1] = 5;
	PoligoniOgrade[7].vertcies[2] = 1;
	PoligoniOgrade[7].vertcies[3] = 2;
	PoligoniOgrade[7].material = &Drvo;

	PoligoniOgrade[8].vertcies[0] = 7;
	PoligoniOgrade[8].vertcies[1] = 6;
	PoligoniOgrade[8].vertcies[2] = 2;
	PoligoniOgrade[8].vertcies[3] = 3;
	PoligoniOgrade[8].material = &Drvo;

	PoligoniOgrade[9].vertcies[0] = 3;
	PoligoniOgrade[9].vertcies[1] = 2;
	PoligoniOgrade[9].vertcies[2] = 1;
	PoligoniOgrade[9].vertcies[3] = 0;
	PoligoniOgrade[9].material = &Drvo;

	TranslateModel3D(&Ograda, NapraviVektor3D(-SirinaOgrade * RazmakIzmedjuStubova / 2, 0, 0));//postavlja stub na levo pocetno mesto ograde najblize kameri

	AddModelToScene(&BozePomozi_Scena, &Ograda);


	///////////////////////////////////////////////namestanje ograde/////////////////////////////////////////////////


	duzinaOgrade--;
	struct ModelInstance* InstanceDasaka = (struct ModelInstance*)calloc(duzinaOgrade * 2 + SirinaOgrade * 2 + 1, sizeof(struct ModelInstance));//instance dasaka
	struct ModelInstance* InstanceStubova = (struct ModelInstance*)calloc(duzinaOgrade * 2 + SirinaOgrade * 2 + 1, sizeof(struct ModelInstance));//instance stubova

	struct Vertex VerteksiInstancaDasaka[16];
	struct Vertex VerteksiInstancaOgrada[12];

	for (int i = 0; i < duzinaOgrade; i++)
	{
		InstanceDasaka[i].ModelReference = &DaskeOgrade;
		InstanceDasaka[i].Translate = NapraviVektor3D(-SirinaOgrade * RazmakIzmedjuStubova / 2, 100, RazmakIzmedjuStubova * (i + 1) + RazmakIzmedjuStubova / 2);
		InstanceDasaka[i].scale = 1;

		AddInstanceToScene(&BozePomozi_Scena, &InstanceDasaka[i]);

		InstanceStubova[i].ModelReference = &Ograda;
		InstanceStubova[i].Translate = NapraviVektor3D(-SirinaOgrade * RazmakIzmedjuStubova / 2, 100, RazmakIzmedjuStubova * (i + 1));
		InstanceStubova[i].scale = 1;

		AddInstanceToScene(&BozePomozi_Scena, &InstanceStubova[i]);
	}

	for (int i = 0; i < SirinaOgrade; i++)
	{
		InstanceDasaka[duzinaOgrade + i].ModelReference = &DaskeOgrade;
		InstanceDasaka[duzinaOgrade + i].Translate = NapraviVektor3D((-SirinaOgrade * RazmakIzmedjuStubova / 2) + RazmakIzmedjuStubova * i + 20 + RazmakIzmedjuStubova / 2, 100, RazmakIzmedjuStubova * (duzinaOgrade + 1));
		InstanceDasaka[duzinaOgrade + i].Rotate = NapraviVektor3D(0, DegreeToRadians(90), 0);
		InstanceDasaka[duzinaOgrade + i].scale = 1;

		AddInstanceToScene(&BozePomozi_Scena, &InstanceDasaka[duzinaOgrade + i]);

		InstanceStubova[duzinaOgrade + i].ModelReference = &Ograda;
		InstanceStubova[duzinaOgrade + i].Translate = NapraviVektor3D((-SirinaOgrade * RazmakIzmedjuStubova / 2) + RazmakIzmedjuStubova * i + 20, 100, RazmakIzmedjuStubova * (duzinaOgrade + 1));
		InstanceStubova[duzinaOgrade + i].scale = 1;

		AddInstanceToScene(&BozePomozi_Scena, &InstanceStubova[duzinaOgrade + i]);
	}

	//AddInstanceToScene(&BozePomozi_Scena, &InstanceStubova[2 * duzinaOgrade + SirinaOgrade + 1]);

	for (int i = 0; i < duzinaOgrade + 1; i++)
	{
		InstanceDasaka[duzinaOgrade + SirinaOgrade + i].ModelReference = &DaskeOgrade;
		InstanceDasaka[duzinaOgrade + SirinaOgrade + i].Translate = NapraviVektor3D(SirinaOgrade * RazmakIzmedjuStubova / 2, 100, RazmakIzmedjuStubova * i + RazmakIzmedjuStubova / 2);
		InstanceDasaka[duzinaOgrade + SirinaOgrade + i].scale = 1;

		AddInstanceToScene(&BozePomozi_Scena, &InstanceDasaka[duzinaOgrade + SirinaOgrade + i]);

		InstanceStubova[duzinaOgrade + SirinaOgrade + i].ModelReference = &Ograda;
		InstanceStubova[duzinaOgrade + SirinaOgrade + i].Translate = NapraviVektor3D(SirinaOgrade * RazmakIzmedjuStubova / 2, 100, RazmakIzmedjuStubova * (i + 1));
		InstanceStubova[duzinaOgrade + SirinaOgrade + i].scale = 1;

		AddInstanceToScene(&BozePomozi_Scena, &InstanceStubova[duzinaOgrade + SirinaOgrade + i]);
	}

	for (int i = 0; i < SirinaOgrade; i++)
	{
		InstanceDasaka[duzinaOgrade * 2 + SirinaOgrade + 1 + i].ModelReference = &DaskeOgrade;
		InstanceDasaka[duzinaOgrade * 2 + SirinaOgrade + 1 + i].Translate = NapraviVektor3D((-SirinaOgrade * RazmakIzmedjuStubova / 2) + RazmakIzmedjuStubova * i + 20 + RazmakIzmedjuStubova / 2, 100, 0);
		InstanceDasaka[duzinaOgrade * 2 + SirinaOgrade + 1 + i].Rotate = NapraviVektor3D(0, DegreeToRadians(90), 0);
		InstanceDasaka[duzinaOgrade * 2 + SirinaOgrade + 1 + i].scale = 1;

		AddInstanceToScene(&BozePomozi_Scena, &InstanceDasaka[duzinaOgrade * 2 + SirinaOgrade + 1 + i]);

		InstanceStubova[duzinaOgrade * 2 + SirinaOgrade + 1 + i].ModelReference = &Ograda;
		InstanceStubova[duzinaOgrade * 2 + SirinaOgrade + 1 + i].Translate = NapraviVektor3D((-SirinaOgrade * RazmakIzmedjuStubova / 2) + RazmakIzmedjuStubova * (i+1) + 20, 100, 0);
		InstanceStubova[duzinaOgrade * 2 + SirinaOgrade + 1 + i].scale = 1;

		AddInstanceToScene(&BozePomozi_Scena, &InstanceStubova[duzinaOgrade * 2 + SirinaOgrade + 1 + i]);
	}

	GraniceKosiliceX = SirinaTravnjaka / 2 - Kosilica.BoundingRadious;
	GraniceKosiliceZ = duzinaTravnjaka - Kosilica.BoundingRadious;

	/////////////////////////////////////////////////Pocetak Igrice//////////////////////////////////////////////


	openwindow(); // otvori graficki prozor
	do {
		/////////////////////////////////////////////game loop/////////////////////////////////////////////////////

		if (kbhit() != 0)
		{
			key = (char)getch();
			if (key == 'w')
			{
					TempPivot = Kosilica.Pivot;
					TempPivot = SaberiVector3D(TempPivot, NapraviVektor3D(BrzinaKosilice* Kosilica.Pravac.x, BrzinaKosilice* Kosilica.Pravac.y, BrzinaKosilice* Kosilica.Pravac.z));

				if (TempPivot.x > GraniceKosiliceX) TempPivot.x = GraniceKosiliceX;
				else if (TempPivot.x < -GraniceKosiliceX) TempPivot.x = -GraniceKosiliceX;
				if (TempPivot.z > GraniceKosiliceZ) TempPivot.z = GraniceKosiliceZ;
				else if (TempPivot.z < Kosilica.BoundingRadious) TempPivot.z = Kosilica.BoundingRadious;


				TranslateModel3D(&Kosilica, OduzmiVector3D(TempPivot, Kosilica.Pivot));
			}
			if (key == 's')
			{
				TempPivot = Kosilica.Pivot;
				TempPivot = SaberiVector3D(TempPivot, NapraviVektor3D(-BrzinaKosilice* Kosilica.Pravac.x, -BrzinaKosilice* Kosilica.Pravac.y, -BrzinaKosilice* Kosilica.Pravac.z));

				if (TempPivot.x > GraniceKosiliceX) TempPivot.x = GraniceKosiliceX;
				else if (TempPivot.x < -GraniceKosiliceX) TempPivot.x = -GraniceKosiliceX;
				if (TempPivot.z > GraniceKosiliceZ) TempPivot.z = GraniceKosiliceZ;
				else if (TempPivot.z < Kosilica.BoundingRadious) TempPivot.z = Kosilica.BoundingRadious;

				TranslateModel3D(&Kosilica, OduzmiVector3D(TempPivot, Kosilica.Pivot));
			}
			if (key == 'a')
			{
				RotateAroundY3D(&Kosilica, DegreeToRadians(10));
			}
			if (key == 'd')
			{
				RotateAroundY3D(&Kosilica, DegreeToRadians(-10));
			}
			if (key == 'e')
			{
				exit = 1;
			}
		}



		//pozicija kamere je iza kosilice
		BozePomozi_Scena.Kamera.Pozicija = NapraviVektor3D((-600 * Kosilica.Pravac.x) + Kosilica.Pivot.x, 200, (-600 * Kosilica.Pravac.z) + Kosilica.Pivot.z);
		BozePomozi_Scena.Kamera.Pravac = Kosilica.Pravac;

		///////////////////////////////////////render faze//////////////////////////////////////////////////

		delay(200);
		clear();

		/////////////2D

		background(BLUE);//nebo
		stroke(NONE);
		fill(GREEN);
		rectangle(-WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2, WINDOW_WIDTH / 2, 0);//trava


		/*kuca
		fill(GREY);
		quadrangle(-350, WISINA_TRAVNJAKA, -300, WISINA_TRAVNJAKA - 3, -300, WISINA_TRAVNJAKA + 40, -350, WISINA_TRAVNJAKA + 37);
		triangle(-300, WISINA_TRAVNJAKA + 40, -325, WISINA_TRAVNJAKA + 65, -350, WISINA_TRAVNJAKA + 37);
		fill(WHITE);
		quadrangle(-300, WISINA_TRAVNJAKA - 3, -300, WISINA_TRAVNJAKA + 40, -270, WISINA_TRAVNJAKA + 33, -270, WISINA_TRAVNJAKA);
		fill(RED);
		quadrangle(-325, WISINA_TRAVNJAKA + 65, -295, WISINA_TRAVNJAKA + 56, -270, WISINA_TRAVNJAKA + 33, -300, WISINA_TRAVNJAKA + 40);
		fill(WHITE);
		quadrangle(-295, WISINA_TRAVNJAKA + 45, -295, WISINA_TRAVNJAKA + 60, -288, WISINA_TRAVNJAKA + 57, -288, WISINA_TRAVNJAKA + 43);
		fill(GREY);
		quadrangle(-304, WISINA_TRAVNJAKA + 53, -304, WISINA_TRAVNJAKA + 61, -295, WISINA_TRAVNJAKA + 60, -295, WISINA_TRAVNJAKA + 45);
		*/
		//sunce
		//RotateVectorAroundY3D(&sunce.Pozicija_Pravac, DegreeToRadians(5));

		////////3D

		DrawScene(&BozePomozi_Scena, frameNumber);

		frameNumber++;
	} while (exit == 0);


	printf("\ndone");
	getch();

	//free memory

	free(InstanceDasaka);
	free(InstanceStubova);
	FreeScene(&BozePomozi_Scena);

	closewindow(); // zatvori graficki prozor
	return 0;
}