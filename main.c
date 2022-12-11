#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

struct pgm{
	int tipo;
	int c;
	int r;
	int mv;
	unsigned char *pData;
};

void readPGMImage(struct pgm *, char *);
void viewPGMImage(struct pgm *);
void writePGMImage(struct pgm *, char *);
void esta_contido(char [],char []);

int main(){
  
  DIR *d, *d_mean;
  char *comp;
	struct pgm img;
  struct dirent *dir;
  struct dirent *dir_mean;
  d = opendir("./images");
  d_mean = opendir("./images");
  rewinddir(d);
  rewinddir(d_mean);
  int cont = 0;
  int k;
  clock_t begin, end;
  double time_per_img, time_total=0;
  // mean -> strcmp("mean", d_name);
  
  if (d){
    //while ((dir = readdir(d)) != NULL){
    for(int i=0;(dir = readdir(d)) != NULL; i+=2){
    
      begin = clock();  //0 -> 3 -> 1 -> 5 -> 9 -> 7 -> 6 -> 10 -> 17 -> 13
      cont +=1;
      
      if (i >= 3){
    
        
        char id[50], nome_arquivo[50], mean[50]="mean", itsmean;
        strncpy(id, dir->d_name + 2, 5);
        strcpy(nome_arquivo, dir->d_name);
        printf("nome do arquivo: %s \n", nome_arquivo);
        printf("id do arquivo: %s \n", id);
        if (strstr(nome_arquivo, mean)!=NULL){
          itsmean = 's';
          printf("tipo mean: %c\n", itsmean);
        } else {
          itsmean = 'n';
          printf("tipo mean: %c\n", itsmean);
        }
        puts("");
        
        /*for(int z=0;(dir_mean = readdir(d_mean))!=NULL;z++){
          if (z>=2){
            printf("%s\n", dir_mean -> d_name);
            comp = strstr(nome_arquivo, "mean");
            if (comp == NULL){
              strcpy(mean, "ok");
            }
          }
       }*/
        
        //printf("id: %s || mean: %s\n", id, mean);
        readPGMImage(&img, dir->d_name);
        //writePGMImage(&img, "saida.pgm");
        //viewPGMImage(&img);
        /*if(strlen(dir->d_name) > 23){
          puts("mean");
        }*/
      }
      end = clock();

      time_per_img = (double)(end - begin) / CLOCKS_PER_SEC;

      time_total += time_per_img;
    }
    closedir(d);
  }
  cont -= 2; //cont fica com o total de arquivos armazenado
  printf("Tempo médio: %lf\n",time_total/cont); //tempo total/total de arquivos(cont)
	printf("Tempo Total: %lf\n",time_total);
	return 0;

}

void readPGMImage(struct pgm *pio, char *filename){

	FILE *fp;
	char ch;
  char diretorio[50] = "images/";

	if (!(fp = fopen(strcat(diretorio, filename),"r"))){
		perror("Erro.");
		exit(1);
	}

	if ( (ch = getc(fp))!='P'){
		puts("A imagem fornecida não está no formato pgm");
		exit(2);
	}
	
	pio->tipo = getc(fp)-48;
	
	fseek(fp,1, SEEK_CUR);

	while((ch=getc(fp))=='#'){
		while( (ch=getc(fp))!='\n');
	}

	fseek(fp,-1, SEEK_CUR);

	fscanf(fp, "%d %d",&pio->c,&pio->r);

	if (ferror(fp)){ 
		perror(NULL);
		exit(3);
	}	
	fscanf(fp, "%d",&pio->mv);
	fseek(fp,1, SEEK_CUR);

	pio->pData = (unsigned char*) malloc(pio->r * pio->c * sizeof(unsigned char));

	switch(pio->tipo){
		case 2:
			puts("Lendo imagem PGM (dados em texto)");
			for (int k=0; k < (pio->r * pio->c); k++){
				fscanf(fp, "%hhu", pio->pData+k);
			}
		break;	
		case 5:
			puts("Lendo imagem PGM (dados em binário)");
			fread(pio->pData,sizeof(unsigned char),pio->r * pio->c, fp);
		break;
		default:
			puts("Não está implementado");
	}
	
	fclose(fp);

}

void writePGMImage(struct pgm *pio, char *filename){
	FILE *fp;
	char ch;

	if (!(fp = fopen(filename,"wb"))){
		perror("Erro.");
		exit(1);
	}

	fprintf(fp, "%s\n","P5");
	fprintf(fp, "%d %d\n",pio->c, pio->r);
	fprintf(fp, "%d\n", 255);

	fwrite(pio->pData, sizeof(unsigned char),pio->c * pio->r, fp);

	fclose(fp);

}


void viewPGMImage(struct pgm *pio){
	printf("Tipo: %d\n",pio->tipo);
	printf("Dimensões: [%d %d]\n",pio->c, pio->r);
	printf("Max: %d\n",pio->mv);
}
