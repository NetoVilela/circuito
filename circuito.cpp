#include<iostream>
#include <fstream>
#include "circuito.h"
#include "string"
#include "bool3S.h"
using namespace std;
///
/// As strings que definem os tipos de porta
///

// Funcao auxiliar que testa se uma string com nome de porta eh valida
// Caso necessario, converte os caracteres da string para maiusculas
bool validType(std::string& Tipo)
{
  if (Tipo.size()!=2) return false;
  Tipo.at(0) = toupper(Tipo.at(0));
  Tipo.at(1) = toupper(Tipo.at(1));
  if (Tipo=="NT" ||
      Tipo=="AN" || Tipo=="NA" ||
      Tipo=="OR" || Tipo=="NO" ||
      Tipo=="XO" || Tipo=="NX") return true;
  return false;
}

// Funcao auxiliar que retorna um ponteiro que aponta para uma porta alocada dinamicamente
// O tipo da porta alocada depende do parametro string de entrada (AN, OR, etc.)
// Caso o tipo nao seja nenhum dos validos, retorna nullptr
// Pode ser utilizada nas funcoes: Circuito::setPort, Circuito::digitar e Circuito::ler
ptr_Port allocPort(std::string& Tipo)
{
  if (!validType(Tipo)) return nullptr;

  if (Tipo=="NT") return new Port_NOT;
  if (Tipo=="AN") return new Port_AND;
  if (Tipo=="NA") return new Port_NAND;
  if (Tipo=="OR") return new Port_OR;
  if (Tipo=="NO") return new Port_NOR;
  if (Tipo=="XO") return new Port_XOR;
  if (Tipo=="NX") return new Port_NXOR;

  // Nunca deve chegar aqui...
  return nullptr;
}

///
/// CLASSE CIRCUITO
///

/// ***********************
/// Inicializacao e finalizacao
/// ***********************

Circuito::Circuito(): Nin(0), Nout(0), Nportas(0){}

Circuito::clear()
{
  Nin = 0;
  Nout = 0;
  Nportas = 0;
  id_out.clear();
  out_circ.clear();
  ports.clear();
}

//falta_fazer();

/// ***********************
/// Funcoes de testagem
/// ***********************

// Retorna true se IdInput eh uma id de entrada do circuito valida (entre -1 e -NInput)
bool Circuito::validIdInput(int IdInput) const
{
  return (IdInput<=-1 && IdInput>=-int(getNumInputs()));
}

// Retorna true se IdOutput eh uma id de saida do circuito valida (entre 1 e NOutput)
bool Circuito::validIdOutput(int IdOutput) const
{
  return (IdOutput>=1 && IdOutput<=int(getNumOutputs()));
}

// Retorna true se IdPort eh uma id de porta do circuito valida (entre 1 e NPort)
bool Circuito::validIdPort(int IdPort) const
{
  return (IdPort>=1 && IdPort<=int(getNumPorts()));
}

// Retorna true se IdOrig eh uma id valida para a origem do sinal de uma entrada de porta ou
// para a origem de uma saida do circuito (podem vir de uma entrada do circuito ou de uma porta)
// validIdOrig == validIdInput OR validIdPort
bool Circuito::validIdOrig(int IdOrig) const
{
  return validIdInput(IdOrig) || validIdPort(IdOrig);
}

// Retorna true se IdPort eh uma id de porta valida (validIdPort) e
// a porta estah definida (estah alocada, ou seja, != nullptr)
bool Circuito::definedPort(int IdPort) const
{
  if (!validIdPort(IdPort)) return false;
  if (ports.at(IdPort-1)==nullptr) return false;
  return true;
}

// Retorna true se IdPort eh uma porta existente (definedPort) e
// todas as entradas da porta com Id de origem valida (usa getId_inPort e validIdOrig)
bool Circuito::validPort(int IdPort) const
{
  if (!definedPort(IdPort)) return false;
  for (unsigned j=0; j<getNumInputsPort(IdPort); j++)
  {
    if (!validIdOrig(getId_inPort(IdPort,j))) return false;
  }
  return true;
}

// Retorna true se o circuito eh valido (estah com todos os dados corretos):
// - numero de entradas, saidas e portas valido (> 0)
// - todas as portas validas (usa validPort)
// - todas as saidas com Id de origem validas (usa getIdOutput e validIdOrig)
// Essa funcao deve ser usada antes de salvar ou simular um circuito
bool Circuito::valid() const
{
  if (getNumInputs()==0) return false;
  if (getNumOutputs()==0) return false;
  if (getNumPorts()==0) return false;
  for (unsigned i=0; i<getNumPorts(); i++)
  {
    if (!validPort(i+1)) return false;
  }
  for (unsigned i=0; i<getNumOutputs(); i++)
  {
    if (!validIdOrig(getIdOutput(i+1))) return false;
  }
  return true;
}

/// ***********************
/// Funcoes de consulta
/// ***********************

unsigned Circuito::getNumInputs(){
  return Nin;
}

unsigned Circuito::getNumOutputs(){
  return id_out.size();
}

unsigned Circuito::getNumPorts(){
  return ports.size();
}

int Circuito::getIdOutput(int IdOutput) const{
  if(validIdOutput(IdOutput)){
    return id_out[IdOutput-1];
  }
  return 0;
}

int Circuito::getOutput(int IdOutput) const{
  if(validIdOutput(IdOutput)){
    return out_circ[IdOutput-1];
  }
  return bool3S::UNDEF;
}

string Circuito::getNamePort(int IdPort) const{
  if(definedPort(IdPort)){
    return ports[IdPort-1]->getName();
  }
  return "??";
}

unsigned Circuito::getNumInputsPort(int IdPort) const{
  if(definedPort(IdPort)){
    return ports[IdPort-1]->getNumInputs();
  }
  return 0;
}

int Circuito::getId_inPort(int IdPort, unsigned I) const{
  if(definedPort(IdPort)){
    return ports[IdPort-1]->getId_in(I);
  }
  return 0;
}

/// ***********************
/// Funcoes de modificacao
/// ***********************

void Circuito::setIdOutput(int IdOut, int IdOrig){
  if(validIdOutput(IdOut) && validIdOrig(IdOrig)){
    id_out[IdOut-1] = IdOrig;
  }
}

void setPort(int IdPort, std::string Tipo, unsigned NIn){
  if(validIdPort(IdPort) && validType(Tipo) && validIdInput(NIn)){
    delete ports[IdPort-1];

    // ports[IdPort-1] <- new Port();
  }
}
//falta_fazer();

/// ***********************
/// E/S de dados
/// ***********************

void Circuito::alloc(unsigned Nentradas, unsigned Nsaidas, unsigned Nport){
    inputs.resize(Nentradas);
    id_out.resize(Nsaidas);
    portas.resize(Nport);
    Nin = Nentradas;
    Nout = Nsaidas;
    Nportas = Nport;
}

void Circuito::digitar()  // Fun��o esta� sendo desenvolvida -Neto Vilela
{
    unsigned Nentradas;
    unsigned Nsaidas;
    unsigned Nport;
    string sigla_porta;
    bool porta_valida;
    do{
        cout << "\n Numero de entradas invalido. Por favor, informe outro\n";
        cin >> Nentradas;
    }while(Nentradas == 0);
    do{
       // cout << "\n Numero de saidas invalido. Por favor, informe outro\n";
        cin >> Nsaidas;
    }while(Nsaidas== 0);
    do{
        cout << "\n Numero de portas invalido. Por favor, informe outro\n";
        cin >> Nport;
    }while(Nport == 0);

    // Redimensiona o circuito
    Nin = Nentradas;
    Nout = Nsaidas;
    Nportas = Nport;

    Porta_NOT NT;
    Porta_AND AN;
    Porta_NAND NA;
    Porta_OR OR;
    Porta_NOR NOR;
    Porta_XOR XO;
    Porta_NXOR NX;

    // L�gica para criar as portas
    for(unsigned i = 0; i< Nport; i++){
        cout << "Portas dispon�veis: (NT,AN,NA,OR,NO,XO,NX) \n";
        cout << "Informe a porta que deseja criar: \n";
        cin >> sigla_porta;

        porta_valida = validType(sigla_porta);
        do{
            cout << "\nA porta digitada eh invalida. Por favor, digite outra porta: \n";
            cout << "Portas disponiveis: (NT,AN,NA,OR,NO,XO,NX) \n";
            cin >> sigla_porta;
            porta_valida = validType(sigla_porta);
        }while(!porta_valida);
      
        if(sigla_porta == "NT") portas[i] = (&NT);
        else if(sigla_porta == "AN") portas[i] = (&AN);
        else if(sigla_porta == "NA") portas[i] = (&NA);
        else if(sigla_porta == "OR") portas[i] = (&OR);
        else if(sigla_porta == "NO") portas[i] = (&NOR);
        else if(sigla_porta == "XO") portas[i] = (&XO);
        else if(sigla_porta == "NX") portas[i] = (&NX);
        else{
            cerr << "Essa porta não existe";
            clear();
            return;
        }
        portas[cont] -> digitar();
        cont++;
  
        int ID;
        for(unsigned i=0; i<Nout; i++)
        {
            cout << "ID do sinal que vai para a saida " << i+1 << endl;
            cin >> ID;
            if(ID <= Nportas){
                id_out[i] = ID;
            }
            else{
                cerr << "id invalido.";
            return;
        }
    }



}

bool Circuito::ler(const std::string& arq){
  ifstream arquivo(arq);
    string prov, tipo;
    int NI, NO, NP, Nin;
    if (arquivo.is_open()){
        arquivo>>prov>>NI>>NO>>NP;
        if(prov!="CIRCUITO:"||NI<=0||NO<=0||NP<=0){
            cerr<<"Erro: Cabecalho 'CIRCUITO:'";
            return;
        }
        clear();
        alloc(NI,NO,NP);
        arquivo.ignore(255,'\n');

        arquivo>>prov;

        if(prov!="PORTAS"){
            cerr<<"Erro: Palavra chave 'PORTAS:'";
            return;
        }
        arquivo.ignore(255,'\n');

        Porta_NOT NT;
        Porta_AND AN;
        Porta_NAND NA;
        Porta_OR OR;
        Porta_NOR NOR;
        Porta_XOR XO;
        Porta_NXOR NX;
        int i=0, int_prov;
        do{
            arquivo >> int_prov;
            if(int_prov != i+1){
                cerr<<"Portas fora de ordem, ou faltando\n";
                return;
            }
            arquivo.ignore(255, ' ');
            arquivo >> tipo;
            if(tipo == "NT") portas[i] = (&NT) -> clone();
            else if(tipo == "AN") portas[i] = (&AN) -> clone();
            else if(tipo == "NA") portas[i] = (&NA) -> clone();
            else if(tipo == "OR") portas[i] = (&OR) -> clone();
            else if(tipo == "NO") portas[i] = (&NOR) -> clone();
            else if(tipo == "XO") portas[i] = (&XO) -> clone();
            else if(tipo == "NX") portas[i] = (&NX) -> clone();  
            else cerr<<"Tipo de porta inexistentes";
            portas[i]->ler(arquivo);
        }   
        while(i < NP);
        arquivo>>prov;
        if(prov!="SAIDAS:"){
            cerr<<"Erro: Palavra chave 'SAIDAS:'";
            return;
        }
        arquivo.ignore(255, '\n');
        i=0;
        do{
            arquivo >> int_prov;
            if(int_prov != i+1){
                cerr<<"Saidas fora de ordem, ou fantando\n";
                return;
            }
            arquivo.ignore(255, ' ');
            arquivo >> int_prov;
            if(int_prov>NP||int_prov==0){
                cerr<<"Id out > Nportas";
                return;
            }
            id_out[i]=int_prov;
            i++;
        }   
        while(i<NO);


    }
};

//falta_fazer();

/// ***********************
/// SIMULACAO (funcao principal do circuito)
/// ***********************

//falta_fazer();
