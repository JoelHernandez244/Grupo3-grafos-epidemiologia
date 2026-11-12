///Autores: Alvarado Méndez Oscar Eduardo (AM22043) 
/// Orellana Velasquez Daniela Estefany (OV22002) 
/// Hernandez Cardenas Joel Eduardo (HC23044)
 ///Fecha: Lunes 2 de noviembre del 2026 
///Hora: 9:00 pm 
/// Descripción: Implementación práctica relacionada con el estudio del uso de grafos
/// en modelos basados en agentes aplicados a la epidemiología. Cada persona se
/// representa como un nodo y sus contactos como conexiones. El programa utiliza
/// listas doblemente enlazadas para representar la red y simula los estados
/// susceptible, infectado y recuperado (modelo SIR).
/// No se utilizan contenedores STL como vector, list o map; sí se emplean
/// componentes de la biblioteca estándar de C++, como string, fstream e iostream.

/// ------------- Librerias utilizadas -------------
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <limits>
#include <cctype>
#include <cstdlib>
#include <ctime>
using namespace std;

/// ---------- Nodo de la lista doblemente enlazada (un contacto) ----------
struct Nodo {
    string id;
    Nodo* siguiente;
    Nodo* anterior;

    Nodo(const string& _id)
        : id(_id), siguiente(nullptr), anterior(nullptr) {}
};

/// ---------- Lista enlazada doble: contactos de una persona ----------
class ListaLED {
private:
    Nodo* cabeza;
    Nodo* cola;
    int tamano;

public:
    ListaLED() : cabeza(nullptr), cola(nullptr), tamano(0) {}

    /// Destructor: libera la memoria de los nodos
    ~ListaLED() { liberarMemoria(); }

    void liberarMemoria() {
        Nodo* actual = cabeza;
        while (actual) {
            Nodo* sig = actual->siguiente;
            delete actual;
            actual = sig;
        }
        cabeza = cola = nullptr;
        tamano = 0;
    }

    int cantidad() const { return tamano; }

    Nodo* obtenerCabeza() const { return cabeza; }

    /// Verifica si un ID ya existe en la lista
    bool existeID(const string& id) const {
        Nodo* actual = cabeza;
        while (actual) {
            if (actual->id == id) return true;
            actual = actual->siguiente;
        }
        return false;
    }

    /// Inserta al final usando el puntero cola
    bool insertarContacto(const string& id) {
        if (existeID(id)) return false;

        Nodo* nuevo = new Nodo(id);
        if (!cabeza) cabeza = cola = nuevo;
        else {
            cola->siguiente = nuevo;
            nuevo->anterior = cola;
            cola = nuevo;
        }
        tamano++;
        return true;
    }

    /// Elimina un contacto por su ID
    bool eliminarContacto(const string& id) {
        Nodo* actual = cabeza;
        while (actual && actual->id != id) actual = actual->siguiente;

        if (!actual) return false;

        if (actual->anterior) actual->anterior->siguiente = actual->siguiente;
        else cabeza = actual->siguiente;

        if (actual->siguiente) actual->siguiente->anterior = actual->anterior;
        else cola = actual->anterior;

        delete actual;
        tamano--;
        return true;
    }

    /// Muestra los contactos de cabeza a cola
    void mostrarContactos() const {
        if (!cabeza) {
            cout << "(sin contactos)";
            return;
        }
        Nodo* actual = cabeza;
        while (actual) {
            cout << actual->id;
            if (actual->siguiente) cout << ", ";
            actual = actual->siguiente;
        }
    }

    /// Muestra los contactos de cola a cabeza
    void mostrarContactosInverso() const {
        if (!cola) {
            cout << "(sin contactos)";
            return;
        }
        Nodo* actual = cola;
        while (actual) {
            cout << actual->id;
            if (actual->anterior) cout << ", ";
            actual = actual->anterior;
        }
    }
};

/// ---------- Persona (vertice de la red) ----------
struct Persona {
    string id;
    string nombre;
    char estado;            // 'S', 'I' o 'R'
    bool marcaInfectar;     // marcas para actualizar los estados al final del dia
    bool marcaRecuperar;
    ListaLED contactos;
    Persona* siguiente;
    Persona* anterior;

    Persona(const string& _id, const string& _nombre, char _estado = 'S')
        : id(_id), nombre(_nombre), estado(_estado),
          marcaInfectar(false), marcaRecuperar(false),
          siguiente(nullptr), anterior(nullptr) {}
};

/// ---------- Red de contactos (lista doble de personas) ----------
class RedContactos {
private:
    Persona* cabeza;
    Persona* cola;
    int total;

    // Devuelve true con una probabilidad p (entre 0 y 1)
    bool hayProbabilidad(double p) const {
        return ((double)rand() / RAND_MAX) < p;
    }

    string textoEstado(char e) const {
        if (e == 'S') return "Susceptible";
        if (e == 'I') return "Infectado";
        return "Recuperado";
    }

public:
    RedContactos() : cabeza(nullptr), cola(nullptr), total(0) {}

    /// Destructor: libera cada persona y su lista de contactos
    ~RedContactos() { liberarMemoria(); }

    void liberarMemoria() {
        Persona* actual = cabeza;
        while (actual) {
            Persona* sig = actual->siguiente;
            delete actual;
            actual = sig;
        }
        cabeza = cola = nullptr;
        total = 0;
    }

    int cantidad() const { return total; }

    Persona* obtenerPersona(const string& id) const {
        Persona* actual = cabeza;
        while (actual) {
            if (actual->id == id) return actual;
            actual = actual->siguiente;
        }
        return nullptr;
    }

    /// Registra una persona nueva al final de la red
    bool agregarPersona(const string& id, const string& nombre, char estado = 'S') {
        if (obtenerPersona(id)) return false;

        Persona* nueva = new Persona(id, nombre, estado);
        if (!cabeza) cabeza = cola = nueva;
        else {
            cola->siguiente = nueva;
            nueva->anterior = cola;
            cola = nueva;
        }
        total++;
        return true;
    }

    /// Elimina una persona y los contactos que otros tenian con ella
    bool eliminarPersona(const string& id) {
        Persona* p = obtenerPersona(id);
        if (!p) return false;

        Nodo* n = p->contactos.obtenerCabeza();
        while (n) {
            Persona* vecino = obtenerPersona(n->id);
            if (vecino) vecino->contactos.eliminarContacto(id);
            n = n->siguiente;
        }

        if (p->anterior) p->anterior->siguiente = p->siguiente;
        else cabeza = p->siguiente;

        if (p->siguiente) p->siguiente->anterior = p->anterior;
        else cola = p->anterior;

        delete p;
        total--;
        return true;
    }

    /// Crea un contacto entre dos personas (se guarda en las dos listas)
    /// Retorna: 0 = ok, 1 = un ID no existe, 2 = mismo ID, 3 = ya son contactos
    int crearContacto(const string& id1, const string& id2) {
        if (id1 == id2) return 2;

        Persona* a = obtenerPersona(id1);
        Persona* b = obtenerPersona(id2);
        if (!a || !b) return 1;
        if (a->contactos.existeID(id2)) return 3;

        a->contactos.insertarContacto(id2);
        b->contactos.insertarContacto(id1);
        return 0;
    }

    /// Rompe el contacto entre dos personas
    bool romperContacto(const string& id1, const string& id2) {
        Persona* a = obtenerPersona(id1);
        Persona* b = obtenerPersona(id2);
        if (!a || !b) return false;

        bool r1 = a->contactos.eliminarContacto(id2);
        bool r2 = b->contactos.eliminarContacto(id1);
        return r1 && r2;
    }

    /// Infecta a una persona (caso inicial)
    bool infectarPersona(const string& id) {
        Persona* p = obtenerPersona(id);
        if (!p || p->estado != 'S') return false;
        p->estado = 'I';
        return true;
    }

    /// Simula un dia con el modelo SIR
    /// beta = probabilidad de contagio por contacto
    /// gamma = probabilidad de que un infectado se recupere
    void simularDia(double beta, double gamma, int& nuevosI, int& nuevosR) {
        nuevosI = 0;
        nuevosR = 0;

        Persona* p = cabeza;
        while (p) {
            p->marcaInfectar = false;
            p->marcaRecuperar = false;
            p = p->siguiente;
        }

        // Primero se decide quien se contagia y quien se recupera
        p = cabeza;
        while (p) {
            if (p->estado == 'I') {
                Nodo* n = p->contactos.obtenerCabeza();
                while (n) {
                    Persona* vecino = obtenerPersona(n->id);
                    if (vecino && vecino->estado == 'S' && hayProbabilidad(beta))
                        vecino->marcaInfectar = true;
                    n = n->siguiente;
                }
                if (hayProbabilidad(gamma)) p->marcaRecuperar = true;
            }
            p = p->siguiente;
        }

        // Despues se aplican los cambios
        p = cabeza;
        while (p) {
            if (p->marcaInfectar) { p->estado = 'I'; nuevosI++; }
            if (p->marcaRecuperar) { p->estado = 'R'; nuevosR++; }
            p = p->siguiente;
        }
    }

    void contarEstados(int& s, int& i, int& r) const {
        s = i = r = 0;
        Persona* p = cabeza;
        while (p) {
            if (p->estado == 'S') s++;
            else if (p->estado == 'I') i++;
            else r++;
            p = p->siguiente;
        }
    }

    /// Muestra todas las personas con sus contactos
    void mostrarRed() const {
        if (!cabeza) {
            cout << "No hay personas registradas.\n";
            return;
        }
        cout << "\nID\tNombre\t\tEstado\t\tContactos\n";
        cout << "-----------------------------------------------------------------\n";
        Persona* p = cabeza;
        while (p) {
            cout << p->id << "\t" << p->nombre << "\t\t" << textoEstado(p->estado)
                 << "\t" << p->contactos.cantidad() << " -> ";
            p->contactos.mostrarContactos();
            cout << "\n";
            p = p->siguiente;
        }
    }

    /// Busca una persona por ID o nombre
    void buscarPersona(const string& valor) const {
        if (!cabeza) {
            cout << "No hay personas registradas.\n";
            return;
        }
        Persona* p = cabeza;
        while (p) {
            if (p->id == valor || p->nombre == valor) {
                cout << "\nPersona encontrada:\n";
                cout << "ID: " << p->id << "\nNombre: " << p->nombre
                     << "\nEstado: " << textoEstado(p->estado)
                     << "\nCantidad de contactos: " << p->contactos.cantidad();
                cout << "\nContactos (directo): ";
                p->contactos.mostrarContactos();
                cout << "\nContactos (inverso): ";
                p->contactos.mostrarContactosInverso();
                cout << "\n";
                return;
            }
            p = p->siguiente;
        }
        cout << "No se encontro ninguna persona con ese ID o nombre.\n";
    }

    /// Guarda la red en un archivo de texto
    /// Formato: P|id|nombre|estado   y   C|id1|id2
    void guardarEnArchivo() const {
        ofstream archivo("red.txt");
        if (!archivo) {
            cout << "Error al guardar el archivo.\n";
            return;
        }
        Persona* p = cabeza;
        while (p) {
            archivo << "P|" << p->id << "|" << p->nombre << "|" << p->estado << "\n";
            p = p->siguiente;
        }
        p = cabeza;
        while (p) {
            Nodo* n = p->contactos.obtenerCabeza();
            while (n) {
                if (p->id < n->id)   // cada contacto se escribe una sola vez
                    archivo << "C|" << p->id << "|" << n->id << "\n";
                n = n->siguiente;
            }
            p = p->siguiente;
        }
        cout << "Red guardada en red.txt\n";
    }

    /// Carga la red desde el archivo
    void cargarDesdeArchivo() {
        ifstream archivo("red.txt");
        if (!archivo) {
            cout << "No se encontro red.txt\n";
            return;
        }
        liberarMemoria();

        string linea;
        while (getline(archivo, linea)) {
            stringstream ss(linea);
            string tipo, dato1, dato2, dato3;
            getline(ss, tipo, '|');
            getline(ss, dato1, '|');
            getline(ss, dato2, '|');
            getline(ss, dato3, '|');

            if (tipo == "P" && !dato1.empty() && !dato2.empty()) {
    char est = dato3.empty() ? 'S' : dato3[0];

    if (est != 'S' && est != 'I' && est != 'R') {
        est = 'S';
    }

    agregarPersona(dato1, dato2, est);
} else if (tipo == "C" && !dato1.empty() && !dato2.empty()) {
    crearContacto(dato1, dato2);
}
        }
        cout << "Red cargada correctamente (" << total << " personas).\n";
    }

    /// Genera una poblacion de ejemplo con contactos al azar
 void generarPoblacionEjemplo(int n, int k) {
    const char* nombres[] = {
        "Ana", "Luis", "Marta", "Jose", "Carla",
        "Pedro", "Sofia", "Diego", "Elena", "Raul"
    };

    string* ids = new string[n];
    int candidato = 1000;

    for (int i = 0; i < n; i++) {
        while (obtenerPersona(to_string(candidato)) != nullptr) {
            candidato++;
        }

        ids[i] = to_string(candidato);
        agregarPersona(ids[i], nombres[i % 10]);
        candidato++;
    }

    for (int i = 0; i < n; i++) {
        for (int c = 0; c < k; c++) {
            int j = rand() % n;
            if (j != i) {
                crearContacto(ids[i], ids[j]);
            }
        }
    }

    delete[] ids;
}
};

/// ---------- Funciones de lectura con validacion ----------
string leerID() {
    string id;
    while (true) {
        cout << "Ingrese ID (solo numeros, minimo 4 digitos): ";
        cin >> id;
        bool valido = !id.empty();
        for (char c : id) if (!isdigit(static_cast<unsigned char>(c))) valido = false;
        if (id.length() >= 4 && valido) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return id;
        }
        cout << "ID invalido.\n";
    }
}

string leerNombre() {
    string nombre;
    while (true) {
        cout << "Ingrese nombre (solo letras): ";
        getline(cin >> ws, nombre);
        bool valido = !nombre.empty();
        for (char c : nombre)
            if (!isalpha(static_cast<unsigned char>(c)) && c != ' ') { valido = false; break; }
        if (valido) return nombre;
        cout << "Nombre invalido.\n";
    }
}

int leerOpcion() {
    int op;
    while (!(cin >> op)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Ingrese solo numeros: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return op;
}

int leerEntero(const string& mensaje, int minimo, int maximo) {
    int valor;
    while (true) {
        cout << mensaje;
        if (cin >> valor && valor >= minimo && valor <= maximo) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return valor;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Valor invalido (de " << minimo << " a " << maximo << ").\n";
    }
}

double leerProbabilidad(const string& mensaje) {
    double p;
    while (true) {
        cout << mensaje;
        if (cin >> p && p >= 0.0 && p <= 1.0) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return p;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Ingrese un valor entre 0 y 1.\n";
    }
}

/// ---------- Menu principal ----------
int main() {
    srand((unsigned)time(nullptr));
    RedContactos red;
    int opcion;

    //------- Carga automatica ----
    cout << "\nIntentando cargar red anterior...\n";
    red.cargarDesdeArchivo();

    do {
        cout << "\n--- SIMULACION DE EPIDEMIA (RED DE CONTACTOS CON LISTA DOBLE ENLAZADA) ---\n";
        cout << "1) Registrar persona\n";
        cout << "2) Eliminar persona\n";
        cout << "3) Crear contacto\n";
        cout << "4) Romper contacto\n";
        cout << "5) Infectar persona (caso inicial)\n";
        cout << "6) Mostrar red\n";
        cout << "7) Buscar persona\n";
        cout << "8) Simular dias\n";
        cout << "9) Resumen de estados\n";
        cout << "10) Guardar red\n";
        cout << "11) Cargar red\n";
        cout << "12) Generar poblacion de ejemplo\n";
        cout << "13) Salir del programa\n";
        cout << "Seleccione una opcion: ";

        opcion = leerOpcion();

        switch (opcion) {
        case 1: {
            string id = leerID();
            string nombre = leerNombre();
            if (red.agregarPersona(id, nombre)) cout << "Persona registrada correctamente.\n";
            else cout << "ID ya registrado.\n";
            break;
        }
        case 2: {
            string id = leerID();
            if (red.eliminarPersona(id)) cout << "Persona eliminada correctamente.\n";
            else cout << "Persona no encontrada.\n";
            break;
        }
        case 3: {
            cout << "Primera persona:\n";  string id1 = leerID();
            cout << "Segunda persona:\n";  string id2 = leerID();
            int r = red.crearContacto(id1, id2);
            if (r == 0) cout << "Contacto creado correctamente.\n";
            else if (r == 1) cout << "Alguno de los IDs no existe.\n";
            else if (r == 2) cout << "Una persona no puede ser contacto de si misma.\n";
            else cout << "Esas personas ya son contactos.\n";
            break;
        }
        case 4: {
            cout << "Primera persona:\n";  string id1 = leerID();
            cout << "Segunda persona:\n";  string id2 = leerID();
            if (red.romperContacto(id1, id2)) cout << "Contacto eliminado correctamente.\n";
            else cout << "Ese contacto no existe.\n";
            break;
        }
        case 5: {
            string id = leerID();
            if (red.infectarPersona(id)) cout << "Persona infectada.\n";
            else cout << "Persona no encontrada o no esta susceptible.\n";
            break;
        }
        case 6:
            red.mostrarRed();
            break;
        case 7: {
            string valor;
            cout << "Ingrese ID o nombre de la persona a buscar: ";
            getline(cin, valor);
            red.buscarPersona(valor);
            break;
        }
        case 8: {
            if (red.cantidad() == 0) {
                cout << "No hay personas registradas.\n";
                break;
            }
            double beta = leerProbabilidad("Probabilidad de contagio por contacto (0 a 1): ");
            double gamma = leerProbabilidad("Probabilidad de recuperacion por dia (0 a 1): ");
            int dias = leerEntero("Cantidad de dias a simular (1 a 100): ", 1, 100);

            cout << "\nDia\tS\tI\tR\tNuevos I\tNuevos R\n";
            for (int d = 1; d <= dias; d++) {
                int nuevosI, nuevosR, s, i, r;
                red.simularDia(beta, gamma, nuevosI, nuevosR);
                red.contarEstados(s, i, r);
                cout << d << "\t" << s << "\t" << i << "\t" << r
                     << "\t" << nuevosI << "\t\t" << nuevosR << "\n";
                if (i == 0) {
                    cout << "Ya no quedan personas infectadas.\n";
                    break;
                }
            }
            break;
        }
        case 9: {
            int s, i, r;
            red.contarEstados(s, i, r);
            cout << "Total: " << red.cantidad() << " | Susceptibles: " << s
                 << " | Infectados: " << i << " | Recuperados: " << r << "\n";
            break;
        }
        case 10:
            red.guardarEnArchivo();
            break;
        case 11:
            red.cargarDesdeArchivo();
            break;
        case 12: {
            int n = leerEntero("Cantidad de personas (2 a 500): ", 2, 500);
            int k = leerEntero("Contactos por persona (1 a 10): ", 1, 10);
            red.generarPoblacionEjemplo(n, k);
            cout << "Poblacion de ejemplo generada.\n";
            break;
        }
        case 13:
            cout << "Guardando red antes de salir....\n";
            red.guardarEnArchivo();
            cout << "Saliendo del programa. Gracias por usar el sistema.\n";
            break;
        default:
            cout << "Opcion invalida.\n";
        }
    } while (opcion != 13);

    return 0;
}
