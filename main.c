#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct pgm {
  int tipo;
  int c;
  int r;
  int mv;
  unsigned char *pData;
};

void readPGMImage(struct pgm *, char *);
void viewPGMImage(struct pgm *);
void writePGMImage(struct pgm *, char *);
char verificarMean(char *);
char * calculaSCM(int *, struct pgm *, struct pgm *, int);
void writeOutputSCM(int *, char *, int );

int main(int argc, char *argv[]) {
  FILE *txt;
  txt = fopen("outscm.txt", "w");
  DIR *d, *d_mean;
  int quantizacao = atoi(argv[1]);
  struct pgm img, img_mean;
  struct dirent *dir=NULL, *dir_mean=NULL;
  d = opendir("./images");
  d_mean = opendir("./images");
  rewinddir(d);
  rewinddir(d_mean);
  int cont = 0;
  int k;
  clock_t begin, end;
  double time_per_img, time_total = 0;
  int *scm=NULL;
  char id[50], nome_arquivo[50], mean[10]="mean",itsmean, nome_arquivo_mean[50], type;
  unsigned char *outtxt;
  if (d) {
    
    for (int i = 0; (dir = readdir(d)) != NULL; i += 2) {

      begin = clock(); 
      cont += 1;

      if (i >= 3) {

        
        type = dir->d_name[0];
        strncpy(id, dir->d_name + 2, 5);
        strcpy(nome_arquivo, dir->d_name);
        itsmean = verificarMean(nome_arquivo);
        if (itsmean == 'n'){
          for(int x=0;(dir_mean=readdir(d_mean))!=NULL;x++){
            if (x>=2){
              if ((strstr(dir_mean->d_name, id)!=NULL) && (strstr(dir_mean->d_name, mean)!=NULL)){
                strcpy(nome_arquivo_mean, dir_mean->d_name);
              }
            }  
          }
          rewinddir(d_mean);
        } else {
          continue;
        }        
        readPGMImage(&img, nome_arquivo);
        char saida[50] = "saida ";
        writePGMImage(&img, saida);
        puts("");
        readPGMImage(&img_mean, nome_arquivo_mean);  
        char saidamean[50] = "saida-mean ";
        writePGMImage(&img_mean, saidamean);      
        outtxt = calculaSCM(scm,&img, &img_mean, quantizacao);
        for(int i=0;i<quantizacao*quantizacao;i++){
          fprintf(txt, "%hhu, ", *(outtxt + i));
        }
        fprintf(txt, "%c \n", type);       
        puts("\n-------------------------------------------"); 
      }

      end = clock();

      time_per_img = (double)(end - begin) / CLOCKS_PER_SEC;

      time_total += time_per_img;
    }
    closedir(d);
    fclose(txt);
  }
  cont -= 2; // cont fica com o total de arquivos armazenado
  printf("Tempo medio: %lf\n",
         time_total / cont); // tempo total/total de arquivos(cont)
  printf("Tempo Total: %lf\n", time_total);
  return 0;
}

void readPGMImage(struct pgm *pio, char *filename) {

  FILE *fp;
  char ch;
  char diretorio[100] = "images/";

  if (!(fp = fopen(strcat(diretorio, filename), "r"))) {
    perror("Erro.");
    exit(1);
  }

  if ((ch = getc(fp)) != 'P') {
    puts("A imagem fornecida não está no formato pgm");
    exit(2);
  }

  pio->tipo = getc(fp) - 48;

  fseek(fp, 1, SEEK_CUR);

  while ((ch = getc(fp)) == '#') {
    while ((ch = getc(fp)) != '\n'){
        fseek(fp, 1, SEEK_CUR);
    }
  }
  

  fseek(fp, -1, SEEK_CUR);

  fscanf(fp, "%d %d", &pio->c, &pio->r);

  if (ferror(fp)) {
    perror(NULL);
    exit(3);
  }
  fscanf(fp, "%d", &pio->mv);
  fseek(fp, 1, SEEK_CUR);

  pio->pData = (unsigned char *)malloc(pio->r * pio->c * sizeof(unsigned char));

  switch (pio->tipo) {
  case 2:
    puts("Lendo imagem PGM (dados em texto)");
    for (int k = 0; k < (pio->r * pio->c); k++) {
      fscanf(fp, "%hhu", pio->pData + k);
    }
    break;
  case 5:
    puts("Lendo imagem PGM (dados em binario)");
    fread(pio->pData, sizeof(unsigned char), pio->r * pio->c, fp);
    break;
  default:
    puts("Não está implementado");
  }

  fclose(fp);
}

void writePGMImage(struct pgm *pio, char *filename) {
  FILE *fp;
  char ch;

  if (!(fp = fopen(filename, "wb"))) {
    perror("Erro.");
    exit(1);
  }

  fprintf(fp, "%s\n", "P5");
  fprintf(fp, "%d %d\n", pio->c, pio->r);
  fprintf(fp, "%d\n", 255);

  fwrite(pio->pData, sizeof(unsigned char), pio->c * pio->r, fp);

  fclose(fp);
}

void viewPGMImage(struct pgm *pio) {
  printf("Tipo: %d\n", pio->tipo);
  printf("Dimensões: [%d %d]\n", pio->c, pio->r);
  printf("Max: %d\n", pio->mv);
}

char verificarMean(char *nome_arquivo){
  char mean[50] = "mean";
  if (strstr(nome_arquivo, mean) != NULL) {
      return 's';
  } else {
      return 'n';
  }
}

char * calculaSCM(int *scm, struct pgm *pioA, struct pgm *pioB, int quant){
  char *output = NULL;
  int linha = pioA->r, coluna = pioA->c;
  unsigned char *pdataA = pioA->pData, *pdataB = pioB->pData;
  int *matrizA = NULL, *matrizB = NULL;
  
  scm = malloc(sizeof(int) * linha * coluna);
  matrizA = malloc(sizeof(int)*linha*coluna);
  matrizB = malloc(sizeof(int)*linha*coluna);
  output = malloc(sizeof(char)*quant*quant);
  
  for(int i = 0; i < linha; i++){
    for(int x = 0; x < coluna; x++){
      *(matrizA + i * coluna + x) = *(pdataA + i * coluna + x) % quant;
      *(matrizB + i * coluna + x) = *(pdataB + i * coluna + x) % quant;
    }
  }

  for(int i = 0; i< quant; i++){
    for(int x = 0; x < quant; x++){
      *(scm + i * quant + x) = 0;
    }
  } 
  for(int i = 0; i< linha; i++){
    for(int x = 0; x < coluna; x++){
      *(scm + *(matrizA + i * coluna + x) * quant + *(matrizB + i * coluna + x)) += 1;
    }
  } 
  for(int i = 0; i< quant; i++){
    for(int x = 0; x < quant; x++){
      *(output + i * quant + x) = *(scm + i * quant + x);
    }
  }
  return output;
}
