#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

// Definição de Callbacks para Linux
void beginCallback(GLenum type) {
    glBegin(type);
}
void endCallback() {
    glEnd();
}
void vertexCallback(GLvoid *data) {
    double *v = (double*)data;
    glVertex2dv(v);
}
void errorCallback(GLenum errorCode) {
    const GLubyte *estring;
    estring = gluErrorString(errorCode);
    fprintf(stderr, "Tessellation Error: %s\n", estring);
}
void combineCallback(GLdouble coords[3], GLdouble *vertex_data[4],
                     GLfloat weight[4], GLdouble **outData)
{
    GLdouble *vertex = (GLdouble *) malloc(6 * sizeof(GLdouble));

    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];

    *outData = vertex;
}

// Definição de Callbacks para Windows
/*void CALLBACK beginCallback(GLenum type) {
    glBegin(type);
}
void CALLBACK endCallback() {
    glEnd();
}
void CALLBACK vertexCallback(void *vertexData) {
    const GLdouble *ptr = (GLdouble *) vertexData;
    glVertex2dv(ptr);
}
void CALLBACK errorCallback(GLenum errorCode) {
    const GLubyte *err = gluErrorString(errorCode);
    printf("Tessellation Error: %s\n", err);
}
void CALLBACK combineCallback(GLdouble coords[3], void *data[4],
                              GLfloat weight[4], void **outData)
{
    GLdouble *vertex = (GLdouble *)malloc(3 * sizeof(GLdouble));
    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];
    *outData = vertex;
}*/

typedef struct {
    float x, y;
} Ponto;

typedef struct {
    int n;
    unsigned char r, g, b;
    Ponto *v;
} Poligono;

typedef struct {
    Poligono bracoEsq;
    Poligono detalheBracoEsq;
    Poligono luvaEsq;
    Poligono bracoDir;
    Poligono detalheBracoDir;
    Poligono luvaDir;
    Poligono pernaEsq;
    Poligono detalhePernaEsq;
    Poligono pernaDir;
    Poligono detalhePernaDir;
    Poligono corpo;
    Poligono detalheCorpo1;
    Poligono detalheCorpo2;
    Poligono rosto;
    Poligono boca;
    Poligono contOlho;
    Poligono iris;
    Poligono cabelo;
} Personagem;

typedef struct {
    Poligono chao;
    Poligono montanhas;
    Poligono estrela;
    Poligono ceu;
    Poligono sol;
    Poligono iglu;
    Poligono sombraIglu;
    Poligono sombra2Iglu;
    Poligono portaIglu;
} Cenario;

// Variáveis para definir posição e ângulo do personagem
float tx = 10.0f, ty = -23.0f;
float anguloGeral = 1.0f;
float escala = 1.5f;

// Ângulos das articulações, usados na animação
float angBracoEsq = 0.0f, angBracoDir = 0.0f;
float angPernaEsq = 0.0f, angPernaDir = 0.0f;
float angCabeca = 0.0f;

// Variável para armazenar posição atual do cenário
float posCenario = -50.0f;

// Função para ler coordenadas de cada polígono a partir de um .txt
Poligono LerPoligono(const char *nomeArquivo) {
    FILE *f = fopen(nomeArquivo, "r");
    if (!f) {
        printf("Erro ao abrir %s!\n", nomeArquivo);
        exit(1);
    }

    Poligono p;
    fscanf(f, "%d %hhu %hhu %hhu", &p.n, &p.r, &p.g, &p.b);
    p.v = malloc(p.n * sizeof(Ponto));
    for (int i = 0; i < p.n; i++)
        fscanf(f, "%f %f", &p.v[i].x, &p.v[i].y);

    fclose(f);
    return p;
}

// Função para desenhar o polígono e o contorno
void DesenhaPoligono(Poligono p) {
    GLUtesselator *tess = gluNewTess();

    // Chamada de Callback Linux
    gluTessCallback(tess, GLU_TESS_BEGIN,  (void (*)()) beginCallback);
    gluTessCallback(tess, GLU_TESS_END,    (void (*)()) endCallback);
    gluTessCallback(tess, GLU_TESS_VERTEX, (void (*)()) vertexCallback);
    gluTessCallback(tess, GLU_TESS_ERROR,  (void (*)()) errorCallback);
    gluTessCallback(tess, GLU_TESS_COMBINE,(void (*)()) combineCallback);

    // Chamadas de Callback Windows
  /*gluTessCallback(tess, GLU_TESS_BEGIN, (void (CALLBACK*)()) beginCallback);
    gluTessCallback(tess, GLU_TESS_END,   (void (CALLBACK*)()) endCallback);
    gluTessCallback(tess, GLU_TESS_VERTEX,(void (CALLBACK*)()) vertexCallback);
    gluTessCallback(tess, GLU_TESS_ERROR, (void (CALLBACK*)()) errorCallback);
    gluTessCallback(tess, GLU_TESS_COMBINE, (void (CALLBACK*)()) combineCallback);*/

    glColor3ub(p.r, p.g, p.b);

    gluTessBeginPolygon(tess, NULL);
    gluTessBeginContour(tess);

    double (*coords)[3] = malloc(p.n * sizeof(double[3]));

    for (int i = 0; i < p.n; i++) {
        coords[i][0] = p.v[i].x;
        coords[i][1] = p.v[i].y;
        coords[i][2] = 0.0;
        gluTessVertex(tess, coords[i], coords[i]);
    }

    gluTessEndContour(tess);
    gluTessEndPolygon(tess);
    gluDeleteTess(tess);
    free(coords);

    glColor3ub(0, 0, 0);
    glLineWidth(1);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < p.n; i++)
        glVertex2f(p.v[i].x, p.v[i].y);
    glEnd();
}

// Desenha cada elemento do cenário na posição correta
void DesenhaCenario(Cenario c){
    DesenhaPoligono(c.ceu);
    glPushMatrix();
        glTranslatef(posCenario, 0.0, 0.0);
        DesenhaPoligono(c.montanhas);
        DesenhaPoligono(c.chao);
        DesenhaPoligono(c.sol);
        DesenhaPoligono(c.estrela);
        DesenhaPoligono(c.iglu);
        DesenhaPoligono(c.sombraIglu);
        DesenhaPoligono(c.sombra2Iglu);
        DesenhaPoligono(c.portaIglu);
        glTranslatef(-200.0f, 0, 0);
        DesenhaPoligono(c.montanhas);
        DesenhaPoligono(c.chao);
        DesenhaPoligono(c.sol);
        DesenhaPoligono(c.estrela);
        DesenhaPoligono(c.iglu);
        DesenhaPoligono(c.sombraIglu);
        DesenhaPoligono(c.sombra2Iglu);
        DesenhaPoligono(c.portaIglu);
    glPopMatrix();
}

// Desenha cada elemento do personagem na posição correta
void DesenhaPersonagem(Personagem p) {
    glPushMatrix();

        glTranslatef(tx, ty, 0);
        glRotatef(anguloGeral, 0, 0, 1);
        glScalef(escala, escala, 1);

        // Braço direito
        glPushMatrix();
            glTranslatef(1.69, 8.23, 0);
            glRotatef(angBracoDir, 0, 0, 1);
            DesenhaPoligono(p.bracoDir);
            DesenhaPoligono(p.detalheBracoDir);
            DesenhaPoligono(p.luvaDir);
        glPopMatrix();

        // Perna direita
        glPushMatrix();
            glTranslatef(0.86, 1.98, 0);
            glRotatef(angPernaDir, 0, 0, 1);
            DesenhaPoligono(p.pernaDir);
            DesenhaPoligono(p.detalhePernaDir);
        glPopMatrix();

        // Corpo
        glTranslatef(0.54, 5.40, 0);
        DesenhaPoligono(p.corpo);
        DesenhaPoligono(p.detalheCorpo1);
        DesenhaPoligono(p.detalheCorpo2);

        // Perna esquerda
        glPushMatrix();
            glTranslatef(0.32, -3.42, 0);
            glRotatef(angPernaEsq, 0, 0, 1);
            DesenhaPoligono(p.pernaEsq);
            DesenhaPoligono(p.detalhePernaEsq);
        glPopMatrix();

        // Braço esquerdo
        glPushMatrix();
            glTranslatef(1.15, 2.83, 0);
            glRotatef(angBracoEsq, 0, 0, 1);
            DesenhaPoligono(p.bracoEsq);
            DesenhaPoligono(p.detalheBracoEsq);
            DesenhaPoligono(p.luvaEsq);
        glPopMatrix();

        // Cabeça
        glPushMatrix();
            glTranslatef(0.81, 4.33, 0);
            glRotatef(angCabeca, 0, 0, 1);
            DesenhaPoligono(p.rosto);
            DesenhaPoligono(p.boca);
            DesenhaPoligono(p.contOlho);
            DesenhaPoligono(p.iris);
            DesenhaPoligono(p.cabelo);
        glPopMatrix();

    glPopMatrix();
}

// Variáveis globais
Personagem personagem;
Cenario cenario;
float t = 0.0f;

// Função para o callback de Timer
// Gera uma animação cíclica, simulando a caminhada
void LoopAndar(int value) {
    t += 0.05f; // Deslocamento por frame
    t = fmodf(t, 2.0f * M_PI); // Zera a variável a cada ciclo [para evitar overflow]
    const float amp = 12.0f; // Amplitude do movimento
    angBracoDir =  amp * sinf(t);
    angBracoEsq = -amp * sinf(t);
    angPernaDir = -amp * sinf(t);
    angPernaEsq =  amp * sinf(t);

    posCenario += 0.1f;

    if (posCenario >= 200.0f){
        posCenario = 0.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, LoopAndar, 0); // Cria uma recursão com um atraso de 16ms
}

void Inicializa(void) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-50, 50, -50, 50);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    DesenhaCenario(cenario);
    DesenhaPersonagem(personagem);
    glutSwapBuffers();
}

// Função para o callback de Reshape
void Redimensionar(int w, int h) {
    if (w == h) {
        glViewport(0, 0, w, h);
    } else if (w > h) {
        int x = (w - h) / 2;
        glViewport(x, 0, h, h);
    } else {
        int y = (h - w) / 2;
        glViewport(0, y, w, w);
    }
}

// Função para ler os .txt referente ao personagem
void getPersonagem(void){
    personagem.bracoEsq = LerPoligono("braco.txt");
    personagem.bracoDir = LerPoligono("braco.txt");
    personagem.detalheBracoEsq = LerPoligono("detalhe_braco.txt");
    personagem.detalheBracoDir = LerPoligono("detalhe_braco.txt");
    personagem.luvaEsq = LerPoligono("luva.txt");
    personagem.luvaDir = LerPoligono("luva.txt");
    personagem.pernaEsq = LerPoligono("perna.txt");
    personagem.pernaDir = LerPoligono("perna.txt");
    personagem.detalhePernaEsq = LerPoligono("detalhe_perna.txt");
    personagem.detalhePernaDir = LerPoligono("detalhe_perna.txt");
    personagem.corpo = LerPoligono("corpo.txt");
    personagem.detalheCorpo1 = LerPoligono("detalhe1_corpo.txt");
    personagem.detalheCorpo2 = LerPoligono("detalhe2_corpo.txt");
    personagem.rosto = LerPoligono("rosto.txt");
    personagem.boca = LerPoligono("boca.txt");
    personagem.contOlho = LerPoligono("contorno_olho.txt");
    personagem.iris = LerPoligono("iris.txt");
    personagem.cabelo = LerPoligono("cabelo.txt");
}

// Função para ler os .txt referente ao cenário
void getCenario(void){
    cenario.estrela = LerPoligono("estrela.txt");
    cenario.ceu = LerPoligono("ceu.txt");
    cenario.sol = LerPoligono("sol.txt");
    cenario.iglu = LerPoligono("iglu.txt");
    cenario.sombraIglu = LerPoligono("sombra_iglu.txt");
    cenario.sombra2Iglu = LerPoligono("sombra2_iglu.txt");
    cenario.portaIglu = LerPoligono("iglu_porta.txt");
    cenario.montanhas = LerPoligono("montanhas.txt");
    cenario.chao = LerPoligono("chao.txt");
}

// Programa principal
int main(int argc, char **argv) {

    getPersonagem();
    getCenario();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1000, 1000);
    glutCreateWindow("Personagem Hierarquico");
    Inicializa();
    glutDisplayFunc(Display);
    glutReshapeFunc(Redimensionar);
    glutTimerFunc(0, LoopAndar, 0);
    glutMainLoop();

    return 0;
}