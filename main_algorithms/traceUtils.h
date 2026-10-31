/*
 * File:   traceUtils.h
 * Author: renzo
 *
 * Utilidades para el modo de traza (--trace <carpeta>): registro iteración por
 * iteración de SA-GA y GRASP en archivos CSV. La traza solo observa la ejecución:
 * no consume números aleatorios ni modifica ningún cálculo de los algoritmos.
 */

#ifndef TRACEUTILS_H
#define TRACEUTILS_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdio>

using namespace std;

// Componentes de la función objetivo de una solución (solo para la traza)
struct FitnessBreakdown {
    double dispatchesCount = 0.0;
    double avgUtilization = 0.0;
    double fulfillmentRatio = 0.0;
    double priorityCoverage = 0.0;
    double overcapacityPenalty = 0.0;
    bool feasible3D = true;
    string weightByVehicle;     // "UT<id>=<peso usado>/<peso máximo>" separados por ";"
};

namespace TraceUtils {
    const vector<string> TRACE_FILES = {
        "saga_population.csv",
        "saga_generations.csv",
        "saga_crossovers.csv",
        "saga_best.csv",
        "grasp_vehicle_selection.csv",
        "grasp_iterations.csv",
        "grasp_layers.csv",
        "grasp_local_search.csv",
        "output_dispatches.csv",
        "output_result_metadata.csv"
    };

    // Borra los archivos de una traza anterior en la carpeta (la carpeta debe existir)
    inline void resetTraceFiles(const string& traceDir) {
        for (const string& name : TRACE_FILES) {
            std::remove((traceDir + "/" + name).c_str());
        }
    }

    // Abre un CSV de traza en modo append; escribe el encabezado si el archivo es nuevo
    inline void openTraceFile(ofstream& file, const string& traceDir, const string& name, const string& header) {
        string path = traceDir + "/" + name;
        ifstream existing(path);
        bool isNew = !existing.good() || existing.peek() == ifstream::traits_type::eof();
        existing.close();

        file.open(path, ios::app);
        file << setprecision(10);
        if (isNew) file << header << "\n";
    }

    inline string breakdownHeader() {
        return "dispatches,avg_utilization,fulfillment_ratio,priority_coverage,weight_penalty_kg,feasible_3d,weight_by_vehicle";
    }

    inline string breakdownToCSV(const FitnessBreakdown& b) {
        ostringstream ss;
        ss << setprecision(10)
           << b.dispatchesCount << ","
           << b.avgUtilization << ","
           << b.fulfillmentRatio << ","
           << b.priorityCoverage << ","
           << b.overcapacityPenalty << ","
           << (b.feasible3D ? 1 : 0) << ","
           << b.weightByVehicle;
        return ss.str();
    }
}

#endif /* TRACEUTILS_H */
