#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <chrono>
#include <thread>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

using namespace std;

#define INT_MAX 9000 // histoire d'être tranquille

// Convertir int en size_t de manière sûre
size_t int2size_t(int val) {
    return (val < 0) ? __SIZE_MAX__ : (size_t)((unsigned)val);
}

// Convertir size_t en int de manière sûre
int size_t2int(size_t val) {
    return (val <= INT_MAX) ? (int)((ssize_t)val) : -1;
}

struct Organisme {
    int positionX;
    int positionY;
    bool vivant;

    Organisme(int x, int y, bool vivant) : positionX(x), positionY(y), vivant(vivant) {}

    // Méthode mise à jour modifiée pour retourner le nouvel état
    bool computeNextState(const std::vector<std::vector<Organisme>>& univers) const {
        int x = positionX;
        int y = positionY;
        int nbVoisinesVivantes = 0;

        // Vérifier les cases adjacentes
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                if (i == 0 && j == 0) continue;
                int nx = x + i;
                int ny = y + j;
                if (nx >= 0 && nx < size_t2int(univers.size()) && ny >= 0 && ny < size_t2int(univers[0].size())) {
                    if (univers[nx][ny].vivant) {
                        nbVoisinesVivantes++;
                    }
                }
            }
        }

        if (vivant) {
            if (nbVoisinesVivantes < 2 || nbVoisinesVivantes > 3) {
                return false; // Mort par isolement ou surpopulation
            } else {
                return true; // Reste vivante
            }
        } else {
            if (nbVoisinesVivantes == 3) {
                return true; // Naissance
            } else {
                return false; // Reste morte
            }
        }
    }
};

// Fonction pour compter le nombre de cellules vivantes
int nombre_vivant(const std::vector<std::vector<Organisme>>& univers) {
    int nb_vivant = 0;
    for (int i = 0; i < size_t2int(univers.size()); ++i) {
        for (int j = 0; j < size_t2int(univers[i].size()); ++j) {
            if (univers[i][j].vivant) {
                nb_vivant++;
            }
        }
    }
    return nb_vivant;
}

// Fonction pour afficher l'univers avec les cellules vivantes et mortes
void afficherUnivers(const std::vector<std::vector<Organisme>>& univers, int tour) {
    // Effacer l'écran et repositionner le curseur
    std::cout << "\033[2J\033[H"; // Commandes ANSI pour effacer l'écran et repositionner le curseur
    std::cout << "Tour " << tour << " | cellules vivantes : " << nombre_vivant(univers) << endl;

    for (int i = 0; i < size_t2int(univers.size()); ++i) {
        for (int j = 0; j < size_t2int(univers[i].size()); ++j) {
            if (univers[i][j].vivant) {
                std::cout << "■"; // cellule vivante
            } else {
                std::cout << " "; // cellule morte □
            }
        }
        std::cout << endl;
    }
    std::cout << endl;
}

// Fonction pour ajouter une cellule vivante à l'univers
void ajouterCellule(std::vector<std::vector<Organisme>>& univers, int x, int y) {
    if (x >= 0 && x < size_t2int(univers.size()) && y >= 0 && y < size_t2int(univers[0].size())) {
        univers[x][y].vivant = true;
    }
}

// Fonction pour obtenir la taille du terminal
void obtenirTailleTerminal(int &rows, int &cols) {
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int ret = GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    if (ret) {
        cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    cols = w.ws_col;
    rows = w.ws_row;
#endif
}

int main(void) {
    srand(time(0)); // pour les déplacements aléatoires

    int m = 0, n = 0, tour = 1;

    // Obtenir la taille du terminal
    obtenirTailleTerminal(m, n);

    // Réduire la taille pour tenir compte de l'affichage
    m -= 3; // Laisser de l'espace pour les infos de tour et des cellules vivantes
    n -= 1; // Laisser de l'espace pour les bords du terminal

    // Initialisation de l'univers
    std::vector<std::vector<Organisme>> univers(m, vector<Organisme>(n, Organisme(0, 0, false)));

    // Initialisation des cellules
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            univers[i][j] = Organisme(i, j, false);
        }
    }

    // Ajouter des cellules initiales
    int population;
    int genere = rand() % ((m * n) - static_cast<int>((m * n) * 0.1) + 1);
    while (genere > m * n || genere < (m * n) * 0.1) {
        genere = rand() % static_cast<int>(m * n);
    }
    population = genere;
    for (int i = 0; i < population; ++i) {
        int x = rand() % m;
        int y = rand() % n;
        ajouterCellule(univers, x, y);
    }

    // Boucle de simulation
    while (true) {
        afficherUnivers(univers, tour);

        // Calculer l'état suivant de chaque cellule
        std::vector<std::vector<bool>> etatSuivant(m, vector<bool>(n));
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                etatSuivant[i][j] = univers[i][j].computeNextState(univers);
            }
        }

        // Appliquer l'état suivant
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                univers[i][j].vivant = etatSuivant[i][j];
            }
        }

        // Pause pour observer l'évolution
        std::this_thread::sleep_for(std::chrono::milliseconds(16));

        // Compter le nombre de cellules vivantes
        if (nombre_vivant(univers) == 0) {
            cout << "Toutes les cellules sont mortes, ajouter ?" << endl;
            cout << "1. Oui" << endl;
            cout << "2. Non" << endl;
            int choix;
            cin >> choix;
            if (choix == 1) {
                cout << "Entrez le nombre de cellules à ajouter : ";
                cin >> population;
                for (int i = 0; i < population; ++i) {
                    int x = rand() % m;
                    int y = rand() % n;
                    ajouterCellule(univers, x, y);
                }
            } else {
                break;
            }
        }

        tour++;
    }

    std::cout << "Appuyez sur Entrée pour fermer le programme...";
    std::cin.ignore();
    std::cin.get(); // ça attend l'entrée de l'utilisateur

    return 0;
}
