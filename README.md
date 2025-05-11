# Sustainable CVRP Optimization with Large-Scale Road Network Data in C++

I conducted this project during my master's degree, under the supervision of Prof. Dr. Jörn Schönberger, Chair of Transport Services and Logistics at TU Dresden, Germany.
This project tackles the **Capacitated Vehicle Routing Problem (CVRP)** with a focus on ** minimizing fuel emissions** using **meta-heuristic algorithms in C++**. It handles real-world large-scale road networks (~2.5GB). Solving CVRP with large real-world data is computationally intensive. Exact methods are infeasible due to the **combinatorial search space**. Therefore, the **local search metaheuristics approach** was used in this case.

## 🏙️ Project Context

The project works on solving the problem: a **logistics provider** must deliver goods to **~100 supermarket locations** (hereafter will be called "delivery request") within the **urban area of Stuttgart, Germany**. The fleet consists of **different vehicle types** (e.g., small, medium, large trucks), each with **distinct fuel consumption characteristics**.

The optimization objective is to:
- **Minimize total fuel consumption**
- **Avoid overloading vehicles** (penalties applied when overloaded)
- The outcomes include the list of delivery requests **assigned** to different trucks, and the **sequences** of those requests.

---

# 🧱 Project Components

The project is structured in **four parts**:

### 1. 📦 Scenario Development
- I used **XML-based scenario files**, which describe the input data including delivery network (nodes, arcs, depot, fleet types, delivery request (quantities,...)). This constructed a basic component of the problem. In this project, I tried the same delivery network with 5 different fleets (each different fleet has different types of trucks, thus different in the fuel consumption profiles)

![Example of 1 xml-based scenario files](https://github.com/user-attachments/assets/bec850db-8cc8-4310-8a41-f303ad7b2f4f)

### 2. 🧮 Optimization Model
- The mathematical model for this project is based on the model developed by Kopfer, H.W., Schönberger, J., & Kopfer, H. Reducing greenhouse gas emissions of a heterogeneous vehicle fleet. Flex Serv Manuf J 26, 221–248 (2014),  [https://doi.org/10.1007/s10696-013-9180-9](https://doi.org/10.1007/s10696-013-9180-9). Besides, the fuel consumption for different truck types was also referenced from thia paper.
  
- The model code is based on the VRP++ application Version 4.02 by the Chair of Transport Services and Logistics, TU Dresden. You can downnload the application at this [link](https://tu-dresden.de/bu/verkehr/ivw/vbl/software/vrp)
  
- From the VRP++ application, I built:
  - Add C++ objects to customize for the case of a heterogeneous fleet
    ![my C++ code in object declaration](https://github.com/user-attachments/assets/307493bf-33c4-488c-af9f-8f61f8af5e61)

  - Model penalty costs for overloads (including penalty on the number of overloaded vehicles and pallets.
    ![my C++ code of calculating the penalties](https://github.com/user-attachments/assets/81e484d9-b004-42ab-a3b1-6520d26a6c97)

  - Modify the objective function for **fuel-based optimization**
    
- The **Vehicle Routing Logic**: Sequential decision making: Assign requests to vehicles, Sequence deliveries per vehicle.
- Local search heuristics with **steepest descent** improve on an initial feasible solution.

### 3. 🌍 Real-Life Road-network Data Integration
- I downloaded data from [OpenStreetMap - Baden-Württemberg, Germany](https://download.geofabrik.de/europe/germany/baden-wuerttemberg.html).
- This data was stored in a local database using **PostgreSQL** and some extensions ( `hstore`: key-value road metadata,  `postgis`: geospatial data types, and `pgrouting`: shortest path calculation).

### 4. 📊 Sensitivity Analysis
To test the algorithm’s performance and **evaluate robustness** across different operating conditions, the project includes a batch run of **405 XML scenario files**. Each scenario varies key model parameters:

| Parameter         | Description                                                                 |
|------------------|-----------------------------------------------------------------------------|
| `Seed`      | Controls random behavior of the algorithm (diversification)                |
| `nbh`        | Size of the neighborhood in local search (exploration extent)              |
| `P1`, `P2`        | Penalties for exceeding vehicle capacity (weight and volume overloads)     |
| Fleet Configuration | 5 Different combinations of heterogeneous vehicle types                    |

![Example of the batch files I created for 405 scenarios](https://github.com/user-attachments/assets/6f6805bc-26c8-4d4d-97b8-d7a23209a227)

---

# 🧱 Project Results:

After successfully building the C++ code and running the batch process, it returned the solution in the text files. This image is an example of one scenario's initial solutions, in which the code returned the solution that assigned 100 delivery requests to 12 trucks, and the sequences of delivery points in each truck. Besides, the total fuel consumption is around 284 liters with total travel lengths of the whole fleet is ~995km.

![](https://github.com/user-attachments/assets/6a047971-eb8f-4c61-bf6f-fa66dab3b6f8)

The local search metaheuristic algorithm improved upon this initial solution, which decreased the objective function is 262.764 liters.

![](https://github.com/user-attachments/assets/c41cb293-df71-4a0a-9b87-b8155a9cf34a)

Across 405 scenarios, the best-performing configuration was:

| Parameter                         | Value                        |
|-----------------------------------|------------------------------|
| **Fleet Used**                    | Fleet 3 (heterogeneous)      |
| **Vehicles Used**                 | 11                           |
| **Random Seed (`S`)**             | 24                           |
| **Penalty Settings (`P1`, `P2`)** | 150 / 250                    |
| **Neighborhood Size (`nbh_size`)**| 50                           |
| **Final Objective (Fuel)**        | **84.00 liters**             |
| **Initial Objective (Fuel)**      | 284.00 liters                |
| **Improvement**                   | ~70%                         |

The result below illustrates this:

![Image of best solution found](https://github.com/user-attachments/assets/8741a493-9b4a-41b6-81ed-ca61c17e62b6)

📸 This best solution was visualized using [uMap](https://wiki.openstreetmap.org/wiki/UMap/Guide)

![Visualization of result of this vehicle routing problem](https://github.com/user-attachments/assets/21de2f20-48ee-4108-8a21-9455888d0ed2)

---

## 📄 License

This project is licensed under the **MIT License**, see the file for details.  
You are free to use, modify, and distribute this code with appropriate credit.

---

## 🤝 Collaboration & Feedback

I'm open to ideas, improvements, and collaborations!  
Feel free to fork, open an issue, or submit a pull request.

📬 Contact: thuynguyen.scm2022@gmail.com  
🔗 LinkedIn: [Thu Thuy Nguyen]([https://linkedin.com/in/your-profile](https://www.linkedin.com/in/thuynguyen-tudresden/))

