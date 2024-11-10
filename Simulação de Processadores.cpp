#include <stdio.h>
#include <conio2.h>
#include <windows.h>
#include <stdlib.h>


#define TCG 100//tamanho char grande
#define TCM 50//char medio
#define TCP 30//char pequeno
#define QTDECPU 4
/*
	Deletar
	Gravar em dispositivo externo
	Gravar em dispositivo interno
	Imprimir
	Ler
*/
//Tarefas
struct TpTarefa{ //Processador, Nome do Arquivo, TipoProcesso, TempoExec (em Ut)
	int proc,TempoExec;
	char NomeArq[TCG],TipoProc[TCP];
	TpTarefa *ant,*prox;
};

//Processador
struct TpProc{
	int ID; //de 1 a 4
	int contTarefaPendente; //conta todas as tarefas que estão no processador
	TpProc *ant,*prox;
	TpTarefa *inicio, *fim;
	int contExecs, contDispInt, contDispEx, contDel, contLer, contImp;//Contador total e os contadores de cada tarefa
	//Gravar em dispositivo interno (maior prioridade); Gravar em dispositivo externo; Deletar; Ler; Imprimir (menor prioridade);
};

struct TpDesc{ //marca o inicio e o fim da lista de processadores
	TpProc *inicio, *fim;
};

char cpuVazio(TpProc *cpu){
	return cpu->contTarefaPendente==0;
}

TpProc *NovoProc(int num){
	//num é o numero do processador
	TpProc *proc = new TpProc;
	
	proc->ant=proc->prox=NULL;
	proc->inicio=proc->fim=NULL;
	proc->ID=num;
	//Atribuindo zero a todos os contadores
	proc->contTarefaPendente = proc->contExecs = proc->contDispInt = proc->contDispEx = proc->contDel = proc->contLer = proc->contImp = 0;
	return proc;
}

void criarCPUs(TpDesc &D,int qtdeCPU){
	TpProc *CPU;
	for(int i=1; i<= qtdeCPU; i++){
		CPU = NovoProc(i);
		if(i==1){
			D.inicio = D.fim = CPU;
		}
		else{
			D.fim->prox = CPU;
			CPU->ant= D.fim;
			D.fim= CPU;
		}
	}
}

TpTarefa *NovaTarefa(int P, char NA[TCG], char TP[TCP], int TE){
	TpTarefa *Bloco = new TpTarefa;
	
	Bloco->ant=Bloco->prox=NULL;
	Bloco->proc=P;
	Bloco->TempoExec=TE;
	strcpy(Bloco->NomeArq,NA);
	strcpy(Bloco->TipoProc,TP);
	
	return Bloco;
};

int getPrioridade(TpTarefa *tarefa){
	//Descobrindo a prioridade a partir da posição do tipo de processo desejado(0 tem maior prioridade, 4 tem menor prioridade)
	char tipoTarefa[5][TCM] = {{"Gravar Dispositivo Interno"}, {"Gravar Dispositivo Externo"}, {"Deletar"}, {"Ler"}, {"Imprimir"}};
	int i=0;
	while(stricmp(tarefa->TipoProc, tipoTarefa[i])!=0 && i < 4){
		i++;
	}
	return i;
}

void inserirTarefa(TpProc *CPU, TpTarefa *novaTarefa){
	//Fila com prioridade
	int pTarefa, pAtual;//p = prioridade
	TpTarefa *tarefaAtual;
	
	if(cpuVazio(CPU)){
		CPU->inicio = CPU->fim = novaTarefa;
		printf("\n\nPrimeira tarefa\n");
	}
	else{
		//Inserindo na ultima posição
		CPU->fim->prox= novaTarefa;
		novaTarefa->ant=CPU->fim;
		CPU->fim= novaTarefa;
		//Descobrindo a prioridade de nova tarefa
		//Posição 0 tem maior prioridade, posição 4 tem menor
		pTarefa= getPrioridade(novaTarefa);
		tarefaAtual= CPU->fim->ant;
		pAtual= getPrioridade(tarefaAtual);
		if(pTarefa < pAtual){
			while(tarefaAtual->ant != NULL && pTarefa < pAtual){
				tarefaAtual= tarefaAtual->ant;
				pAtual= getPrioridade(tarefaAtual);
			}
			if(tarefaAtual->prox != novaTarefa){
				//Inserindo no meio
				//No caso de inicio, o inicio está sendo executado, então inserir depois de inicio, ou seja, meio.
				CPU->fim= CPU->fim->ant;
				CPU->fim->prox = NULL;
				tarefaAtual->prox->ant= novaTarefa;
				novaTarefa->ant = tarefaAtual;
				novaTarefa->prox= tarefaAtual->prox;
				tarefaAtual->prox = novaTarefa;
				printf("\n\nInserido no começo\n\n");
			}
		}
		/*if(novaTarefa->prox==NULL){
			printf("\n\nContinua no fim\n");
		}*/
	}
	//getch();
	CPU->contTarefaPendente++;
}

void removerTarefa(TpProc *cpu){
	TpTarefa *tarefaRemover= cpu->inicio;
	cpu->contTarefaPendente--;
	if(cpuVazio(cpu)){
		cpu->inicio = cpu->fim = NULL;
	}
	else{
		cpu->inicio=cpu->inicio->prox;
		cpu->inicio->ant= NULL;
	}
	tarefaRemover->prox= NULL;
	delete tarefaRemover;
}

void Inicializar(TpDesc &Desc){
	Desc.inicio=Desc.fim=NULL;
}

char verificaArq(char nomeArq[TCG]){
	system("cls");
	FILE *ptr = fopen(nomeArq,"r");
	if(ptr!=NULL){
		printf("O arquivo '%s' existe e esta pronto para ser utilizado.\n\n", nomeArq);
		printf("Pressione qualquer tecla para prosseguir.\n\n");
		getch();
		fclose(ptr);
		return 1;
	}
	else{
		printf("O arquivo '%s' nao existe.\n\n", nomeArq);
		printf("Pressione qualquer tecla para prosseguir.");
		getch();
		fclose(ptr);
		return 0;
	}
}

void CriaBackup(TpDesc Desc,FILE *leitura){ //Geração do arquivo de backup em caso de abortar o programa
	FILE *arq = fopen("ArquivoBackup.txt","w+");
	int numProc,tempoExec;
	char nomeArq[TCG], tipoProcesso[TCP];
	//while(Desc.inicio!=NULL){
		TpProc *atualCPU=Desc.inicio;
		while(atualCPU!=NULL){ //vai passar por todos os processadores
			//O total de tarefas executadas por cada Processador
			//ID do processador, Quantidade de tarefas executadas + as que faltaram
			fprintf(arq,"%d,%d\n",atualCPU->ID, atualCPU->contExecs + atualCPU->contTarefaPendente);//contTarefaPendente não vai ser sempre zero?
			
			//O total individual por tipo de tarefa solicitada
			//Internos, Externos, Delete, Ler, Imprimir
			fprintf(arq,"%d,%d,%d,%d,%d\n",atualCPU->contDispInt, atualCPU->contDispEx, atualCPU->contDel, atualCPU->contLer, atualCPU->contImp);
			atualCPU=atualCPU->prox;
		}
	while(!feof(leitura)){
		fscanf(leitura, "%d,%[^,],%[^,],%d\n", &numProc, &nomeArq, &tipoProcesso, &tempoExec);
		fprintf(arq, "%d,%s,%s,%d\n", numProc, nomeArq, tipoProcesso, tempoExec);
	}
	printf("\n\nArquivo Backup gerado!\n");
	fclose(arq);
}

void Moldura(int CI, int LI, int CF, int LF, int Cor, int Fundo)
{
	int i;
	textcolor(Cor);
	textbackground(Fundo);
	gotoxy(CI, LI);
	printf("%c",201);
	gotoxy(CF,LI);
	printf("%c",187);
	gotoxy(CI,LF);
	printf("%c",200);
	gotoxy(CF,LF);
	printf("%c",188);
	
	for (i=CI+1; i<CF; i++)
	{
		gotoxy(i,LI);
		printf("%c",205);
		gotoxy(i,LF);
		printf("%c",205);
	}
	for (i=LI+1; i<LF; i++)
	{
		gotoxy(CI,i);
		printf("%c",186);
		gotoxy(CF,i);
		printf("%c",186);
	}
	
	textcolor(7);
	textbackground(0);
}

void Exibe(TpDesc Desc){
	//Largura máx por proc 50
	system("cls");
	int i=0,j=0;
	TpProc *atualP = Desc.inicio;
	TpTarefa *atualT;
	while(atualP!=NULL)
	{
		Moldura(5+(30*i),3,30+(30*i),25,16,7);
		Moldura(5+(30*i),3,30+(30*i),7,16,7);
		gotoxy(11+(30*i),4);
		printf("Processador: %d",atualP->ID);
		gotoxy(8+(30*i),5);
		printf("Tarefas Pendentes: %d",atualP->contTarefaPendente);
		atualT=atualP->inicio;
		while(atualT!=NULL)
		{
			gotoxy(8+(30*i),8+(1*j));
			printf("%s - %s",atualT->NomeArq,atualT->TipoProc);
			atualT=atualT->prox;
			j++;
		}
		j=0;
		i++;
		atualP=atualP->prox;
	}
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
void ExibeTeste(TpDesc &Desc){
	TpProc *atualCPU= Desc.inicio;
	TpTarefa *atualTarefa;	
	while(atualCPU!=NULL){
		printf("\n\nCPU: %d ContTarefasPendentes: %d\n", atualCPU->ID, atualCPU->contTarefaPendente);
		printf("ContExecs: %d - ContDispInt: %d - ContDispEx: %d - ContDel: %d - ContLer: %d - ContImp: %d\n", atualCPU->contExecs, atualCPU->contDispInt, atualCPU->contDispEx, atualCPU->contDel, atualCPU->contLer, atualCPU->contImp);
		atualTarefa= atualCPU->inicio;
		printf("Tarefas: \n");
		while(atualTarefa!=NULL){
			
			printf("%d - %s - %s - %d\n", atualTarefa->proc, atualTarefa->NomeArq, atualTarefa->TipoProc, atualTarefa->TempoExec);
			atualTarefa= atualTarefa->prox;
		}
		atualCPU= atualCPU->prox;
	}
}

void executarTarefas(TpDesc &Desc){
	TpProc *atualCPU= Desc.inicio;
	while(atualCPU != NULL){
		if(!cpuVazio(atualCPU)){					
			atualCPU->inicio->TempoExec--;
			if(atualCPU->inicio->TempoExec == 0){
				atualCPU->contExecs++;
				switch(getPrioridade(atualCPU->inicio)){
					case 0: atualCPU->contDispInt++;
						break;
					case 1: atualCPU->contDispEx++;
						break;
					case 2: atualCPU->contDel++;
						break;
					case 3: atualCPU->contLer++;
						break;
					case 4: atualCPU->contImp++;
						break;
				}
				removerTarefa(atualCPU);//A função já decrementa o contTarefaPendente;
			}
		}
		atualCPU=atualCPU->prox;
	}
}

int main(void){
	int ut= 1,i, numProc, tempoExec, contCpuSemTarefa= 0;
	char tecla='A', nomeArq[TCG], tipoProcesso[TCP];
	TpDesc Desc;
	TpProc *atualCPU, *execCPU;
	TpTarefa *tarefa, *atualTarefa;
	
	Inicializar(Desc);
	criarCPUs(Desc, QTDECPU);
	strcpy(nomeArq, "");
	if(verificaArq("ArquivoBackup.txt")){
		strcpy(nomeArq, "ArquivoBackup.txt");
	}
	else{
		if(verificaArq("Trabalho Pratico  ED I  1 bimestre  1 Semestre 2023  Processadores.txt"))
			strcpy(nomeArq, "Trabalho Pratico  ED I  1 bimestre  1 Semestre 2023  Processadores.txt");
	}
	if(stricmp(nomeArq, "") != 0){//Se nomeArq != vazio
		
		FILE*leitura = fopen(nomeArq,"r");
		if(stricmp(nomeArq, "ArquivoBackup.txt")==0){
			//Restaurar os dados anteriores(definir um padrão, para inserir e retirar os dados)
			atualCPU= Desc.inicio;
			while(atualCPU!=NULL){
				fscanf(leitura, "%d, %d\n", &atualCPU->ID, &atualCPU->contExecs);
				fscanf(leitura, "%d,%d,%d,%d,%d\n", &atualCPU->contDispInt, &atualCPU->contDispEx, &atualCPU->contDel, &atualCPU->contLer, &atualCPU->contImp);
				atualCPU= atualCPU->prox;
			}
		}
		//Continuar lendo
		do{
			if(ut % 2 == 0){
				fscanf(leitura, "%d,%[^,],%[^,],%d\n", &numProc, &nomeArq, &tipoProcesso, &tempoExec);
				atualCPU= Desc.inicio;
				while(numProc != atualCPU->ID){
					atualCPU= atualCPU->prox;
				}
				//Cadastrar tarefa
				tarefa = NovaTarefa(numProc, nomeArq, tipoProcesso, tempoExec);
				inserirTarefa(atualCPU, tarefa);
			}
			if(kbhit()){
				tecla = getch();//Terminar todos os processos e Passar informações para novo arq
			}
			//Executando as tarefas
			system("cls");
			executarTarefas(Desc);
			ExibeTeste(Desc);
			ut++;
			Sleep(500);
		}while(!feof(leitura) && tecla != 27);
		
		//Executando as tarefas enquanto há tarefas em algum cpu
		while(contCpuSemTarefa != QTDECPU){
			contCpuSemTarefa= 0;
			executarTarefas(Desc);
			atualCPU=Desc.inicio;
			while(atualCPU!=NULL){
				if(cpuVazio(atualCPU))
					contCpuSemTarefa++;
				atualCPU=atualCPU->prox;
			}
			system("cls");
			ExibeTeste(Desc);
			Sleep(500);
			ut++;//Precisa?
		}
		if(tecla==27)
			CriaBackup(Desc, leitura);
		fclose(leitura);
	}
	return 0;
}
