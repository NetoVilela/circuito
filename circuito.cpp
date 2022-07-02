#include <iostream>
#include <fstream>
#include "circuito.h"
#include "string"
#include "bool3S.h"
#include "port.h"

// Bernardo Fonseca Andrade de Lima
// Francisco de Assis Vilela Neto

using namespace std;
///
/// As strings que definem os tipos de porta
///

// Funcao auxiliar que testa se uma string com nome de porta eh valida
// Caso necessario, converte os caracteres da string para maiusculas
bool validType(std::string &Tipo)
{
  if (Tipo.size() != 2)
    return false;
  Tipo.at(0) = toupper(Tipo.at(0));
  Tipo.at(1) = toupper(Tipo.at(1));
  if (Tipo == "NT" ||
      Tipo == "AN" || Tipo == "NA" ||
      Tipo == "OR" || Tipo == "NO" ||
      Tipo == "XO" || Tipo == "NX")
    return true;
  return false;
}

// Funcao auxiliar que retorna um ponteiro que aponta para uma porta alocada dinamicamente
// O tipo da porta alocada depende do parametro string de entrada (AN, OR, etc.)
// Caso o tipo nao seja nenhum dos validos, retorna nullptr
// Pode ser utilizada nas funcoes: Circuito::setPort, Circuito::digitar e Circuito::ler
ptr_Port allocPort(std::string &Tipo)
{
  if (!validType(Tipo))
    return nullptr;

  if (Tipo == "NT")
    return new Port_NOT;
  if (Tipo == "AN")
    return new Port_AND;
  if (Tipo == "NA")
    return new Port_NAND;
  if (Tipo == "OR")
    return new Port_OR;
  if (Tipo == "NO")
    return new Port_NOR;
  if (Tipo == "XO")
    return new Port_XOR;
  if (Tipo == "NX")
    return new Port_NXOR;

  // Nunca deve chegar aqui...
  return nullptr;
}

///
/// CLASSE CIRCUITO
///

/// ***********************
/// Inicializacao e finalizacao
/// ***********************

Circuito::Circuito() : Nin(0), Nout(0), Nportas(0) {}

void Circuito::clear()
{
  Nin = 0;
  Nout = 0;
  Nportas = 0;
  id_out.clear();
  out_circ.clear();
  ports.clear();
}

Circuito::~Circuito() { clear(); }

void Circuito::resize(unsigned Nentradas, unsigned Nsaidas, unsigned Nport)
{
  if (!Nentradas > 0 && !Nsaidas > 0 && !Nport > 0)
  {
    cout << "parametros de Resize invalidos";
    return;
  }
  clear();
  Nin = Nentradas;
  Nout = Nsaidas;
  Nportas = Nport;
  id_out.resize(Nsaidas);
  ports.resize(Nport);

  for (unsigned i = 0; i < getNumOutputs(); i++)
  {
    id_out[i] = 0;
    out_circ[i] = bool3S::UNDEF;
  }

  for (unsigned i = 0; i < getNumPorts(); i++)
    ports[i] = nullptr;
}

/// ***********************
/// Funcoes de testagem
/// ***********************

// Retorna true se IdInput eh uma id de entrada do circuito valida (entre -1 e -NInput)
bool Circuito::validIdInput(int IdInput) const
{
  return (IdInput <= -1 && IdInput >= -int(getNumInputs()));
}

// Retorna true se IdOutput eh uma id de saida do circuito valida (entre 1 e NOutput)
bool Circuito::validIdOutput(int IdOutput) const
{
  return (IdOutput >= 1 && IdOutput <= int(getNumOutputs()));
}

// Retorna true se IdPort eh uma id de porta do circuito valida (entre 1 e NPort)
bool Circuito::validIdPort(int IdPort) const
{
  return (IdPort >= 1 && IdPort <= int(getNumPorts()));
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
  if (!validIdPort(IdPort))
    return false;
  if (ports.at(IdPort - 1) == nullptr)
    return false;
  return true;
}

// Retorna true se IdPort eh uma porta existente (definedPort) e
// todas as entradas da porta com Id de origem valida (usa getId_inPort e validIdOrig)
bool Circuito::validPort(int IdPort) const
{
  if (!definedPort(IdPort))
    return false;
  for (unsigned j = 0; j < getNumInputsPort(IdPort); j++)
  {
    if (!validIdOrig(getId_inPort(IdPort, j)))
      return false;
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
  if (getNumInputs() == 0)
    return false;
  if (getNumOutputs() == 0)
    return false;
  if (getNumPorts() == 0)
    return false;
  for (unsigned i = 0; i < getNumPorts(); i++)
  {
    if (!validPort(i + 1))
      return false;
  }
  for (unsigned i = 0; i < getNumOutputs(); i++)
  {
    if (!validIdOrig(getIdOutput(i + 1)))
      return false;
  }
  return true;
}

/// ***********************
/// Funcoes de consulta
/// ***********************

unsigned Circuito::getNumInputs() const
{
  return Nin;
}

unsigned Circuito::getNumOutputs() const
{
  return id_out.size();
}

unsigned Circuito::getNumPorts() const
{
  return ports.size();
}

int Circuito::getIdOutput(int IdOutput) const
{
  if (validIdOutput(IdOutput))
  {
    return id_out[IdOutput - 1];
  }
  return 0;
}

bool3S Circuito::getOutput(int IdOutput) const
{
  if (validIdOutput(IdOutput))
  {
    return out_circ[IdOutput - 1];
  }
  return bool3S::UNDEF;
}

string Circuito::getNamePort(int IdPort) const
{
  if (definedPort(IdPort))
  {
    return ports[IdPort - 1]->getName();
  }
  return "??";
}

unsigned Circuito::getNumInputsPort(int IdPort) const
{
  if (definedPort(IdPort))
  {
    return ports[IdPort - 1]->getNumInputs();
  }
  return 0;
}

int Circuito::getId_inPort(int IdPort, unsigned I) const
{
  if (definedPort(IdPort))
  {
    return ports[IdPort - 1]->getId_in(I);
  }
  return 0;
}

/// ***********************
/// Funcoes de modificacao
/// ***********************

void Circuito::setIdOutput(int IdOut, int IdOrig)
{
  if (validIdOutput(IdOut) && validIdOrig(IdOrig))
  {
    id_out[IdOut - 1] = IdOrig;
  }
}

void Circuito::setPort(int IdPort, std::string Tipo, unsigned NIn)
{
  if (validIdPort(IdPort) && validType(Tipo) && validIdInput(NIn))
  {
    delete ports[IdPort - 1];

    ports[IdPort - 1] = allocPort(Tipo);

    ports[IdPort - 1]->setNumInputs(NIn);
  }
}

void Circuito::setId_inPort(int IdPort, unsigned I, int IdOrig) const
{
  if (definedPort(IdPort) && ports[IdPort - 1]->validIndex(I) && validIdOrig(IdOrig))
    ports[IdPort - 1]->setId_in(I, IdOrig);
}

// falta_fazer();

/// ***********************
/// E/S de dados
/// ***********************

void Circuito::digitar()
{
  unsigned Nentradas;
  unsigned Nsaidas;
  unsigned Nport;
  string sigla_porta;
  bool porta_valida;
  do
  {
    cout << "\n Numero de entradas invalido. Por favor, informe outro\n";
    cin >> Nentradas;
  } while (Nentradas == 0);
  do
  {
    cout << "\n Numero de saidas invalido. Por favor, informe outro\n";
    cin >> Nsaidas;
  } while (Nsaidas == 0);
  do
  {
    cout << "\n Numero de portas invalido. Por favor, informe outro\n";
    cin >> Nport;
  } while (Nport == 0);

  // Redimensiona o circuito
  resize(Nentradas, Nsaidas, Nport);

  Port_NOT NT;
  Port_AND AN;
  Port_NAND NA;
  Port_OR OR;
  Port_NOR NOR;
  Port_XOR XO;
  Port_NXOR NX;

  for (unsigned i = 0; i < Nport; i++)
  {
    cout << "Portas disponiveis: (NT,AN,NA,OR,NO,XO,NX) \n";
    cout << "Informe a porta que deseja criar: \n";
    cin >> sigla_porta;

    do
    {
      cout << "\nA porta digitada eh invalida. Por favor, digite outra porta: \n";
      cout << "Portas disponiveis: (NT,AN,NA,OR,NO,XO,NX) \n";
      cin >> sigla_porta;
    } while (!validType(sigla_porta));

    if (sigla_porta == "NT")
      ports[i] = (&NT)->clone();
    else if (sigla_porta == "AN")
      ports[i] = (&AN)->clone();
    else if (sigla_porta == "NA")
      ports[i] = (&NA)->clone();
    else if (sigla_porta == "OR")
      ports[i] = (&OR)->clone();
    else if (sigla_porta == "NO")
      ports[i] = (&NOR)->clone();
    else if (sigla_porta == "XO")
      ports[i] = (&XO)->clone();
    else if (sigla_porta == "NX")
      ports[i] = (&NX)->clone();
    else
    {
      cout << "Erro: Essa porta não existe. ";
      clear();
      return;
    }
    ports[i]->digitar();

    int ID;
    for (unsigned i = 0; i < Nout; i++)
    {
      cout << "ID do sinal que vai para a saida " << i + 1 << endl;
      cin >> ID;
      if (validIdOrig(ID))
      {
        id_out[i] = ID;
      }
      else
      {
        do
        {
          cout << "ID invalido. Por favor, digite outro ID do sinal. \n";
          cin >> ID;
        } while (!validIdOrig(ID));
        id_out[i] = ID; // Se chegou aqui, eh, pq o ID ja estah validado
      }
    }
  }
}

bool Circuito::ler(const std::string &arq)
{
  ifstream arquivo(arq);
  string prov, tipo;
  int NI, NO, NP, Nin;
  if (arquivo.is_open())
  {
    arquivo >> prov >> NI >> NO >> NP;
    if (prov != "CIRCUITO:" || NI <= 0 || NO <= 0 || NP <= 0)
    {
      cerr << "Erro: Cabecalho fora do padrao esperado.\n";
      return false;
    }

    resize(NI, NO, NP);
    arquivo.ignore(255, '\n');

    arquivo >> prov;

    if (prov != "PORTAS")
    {
      cout << "Erro: Palavra chave 'PORTAS:'";
      return false;
    }
    arquivo.ignore(255, '\n');

    Port_NOT NT;
    Port_AND AN;
    Port_NAND NA;
    Port_OR OR;
    Port_NOR NOR;
    Port_XOR XO;
    Port_NXOR NX;
    int i = 0, int_prov;
    do
    {
      arquivo >> int_prov;
      if (int_prov != i + 1)
      {
        cout << "Portas faltando ou nao estao ordenadas\n";
        return false;
      }
      arquivo.ignore(255, ' ');
      arquivo >> tipo;

      if (!validType(tipo))
      {
        cout << "Tipo de porta invalido. Por favor, verifique o arquivo e tente novamente. \n";
      }

      if (tipo == "NT")
        ports[i] = (&NT)->clone();
      else if (tipo == "AN")
        ports[i] = (&AN)->clone();
      else if (tipo == "NA")
        ports[i] = (&NA)->clone();
      else if (tipo == "OR")
        ports[i] = (&OR)->clone();
      else if (tipo == "NO")
        ports[i] = (&NOR)->clone();
      else if (tipo == "XO")
        ports[i] = (&XO)->clone();
      else if (tipo == "NX")
        ports[i] = (&NX)->clone();
      else
        cout << "Tipo de porta inexistentes";

      if (!ports[i]->ler(arquivo))
      {
        cout << "Erro: Não foi possivel ler o arquivo para a porta i = " << i + 1 << endl;
        return false;
      }

    } while (i < NP);

    arquivo >> prov;
    if (prov != "SAIDAS:")
    {
      cout << "Erro: Palavra chave 'SAIDAS:'";
      return false;
    }
    arquivo.ignore(255, '\n');

    i = 0;
    for (unsigned i = 0; i < NO; i++)
    {
      arquivo >> int_prov;
      if (int_prov != i + 1)
      {
        cout << "Saidas fora de ordem, ou fantando\n";
        return false;
      }
      arquivo.ignore(255, ' ');

      arquivo >> int_prov; // Não sei se deve ser lido de novo
      if (!validIdPort(NP))
      {
        cout << "Erro: Id out > Nportas";
        return false;
      }

      id_out[i] = int_prov;
    }
  }

  return true;
};

// falta_fazer();

/// ***********************
/// SIMULACAO (funcao principal do circuito)
/// ***********************

bool Circuito::simular(const std::vector<bool3S> &in_circ)
{
  bool tudo_def, alguma_def;
  vector<bool3S> in_port;

  for (unsigned i = 0; i < Nportas; i++)
  {
    ports[i]->setOutput(bool3S::UNDEF);
  }

  do
  {
    tudo_def = true;
    alguma_def = false;

    for (unsigned i = 0; i < Nportas; i++)
    {
      if (ports[i]->getOutput() == bool3S::UNDEF)
      {

        for (unsigned j = 0; j < ports[i]->getNumInputs(); i++)
        {
          if (ports[i]->getId_in(j) > 0){
            in_port[j] = ports[ports[i]->getId_in(j) - 1]->getOutput();
          }
          else {
            in_port[j] = in_circ[-1 * (ports[i]->getId_in(j)) - 1];
          }
        }

        ports[i]->simular(in_port);

        if (ports[i]->getOutput() == bool3S::UNDEF)
        {
          tudo_def = false;
        }
        else
        {
          alguma_def = true;
        }
      }
    }
  } while (!tudo_def && alguma_def);
}
