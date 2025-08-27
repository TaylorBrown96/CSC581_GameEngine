# CSC581_GameEngine

## 📖 Project Introduction
The **Game Engine Construction Project** is designed to provide comprehensive, hands-on experience in the **design, development, integration, and testing** of a functional game engine.  
In addition to team-based engine development, each student will implement an **individual game** using the shared engine to showcase system functionalities.  

This project emphasizes:
- Game engine architecture  
- System implementation and integration  
- Problem-solving in real-world game development scenarios  


## 🎯 Project Purpose
By the end of the semester, you will:
- Develop a **modular, reusable game engine** capable of supporting multiple game genres.  
- Demonstrate system functionality via **individual mini-games**.  
- Reflect on your contributions, design decisions, and challenges through short papers.  
- Gain experience in **collaborative development workflows** using industry-standard tools.  


## 🗂️ Overall Format
- **Semester-long, team-based project** with five graded milestones.  
- Each milestone includes both **team contributions** (engine development) and **individual deliverables** (mini-games + reflection papers).  
- All submissions must comply with the **University’s Academic Integrity Policy**.  


## 🧑‍💻 Learning Outcomes
This project supports the following competencies:

- **CC1**: Define the core architectural components and layers of a game engine.  
- **CC2**: Design and implement a game engine with integrated, communicating systems.  
- **CC3**: Evaluate engine designs for varied game genres and needs.  
- **CC4**: Implement systems supporting varied game functionalities.  
- **CC5**: Implement a conceptual game object model using software object modeling.  
- **CC6**: Test and troubleshoot integrated engine systems.  
- **CC7**: Build and refine a general-purpose game engine.  
- **CC8**: Develop multiple games using a custom-built engine.  


## 📌 Project Scenario & Process
Your team has been tasked with creating a **modular, reusable engine** that supports rapid development for multiple game genres.  
The engine will be built through **five major milestones**, each focusing on specific system features.  

### Milestones
1. **Build the Game Engine Foundations** (Modules 1–3)  
   - Core architecture, gameplay frameworks, and loop structures.  

2. **Integrate Conceptual Models** (Modules 4–6)  
   - Networking models, time management, and multithreading capabilities.  

3. **Implement Runtime Object Models** (Modules 7–9)  
   - Runtime object models using object-centric or property-centric approaches.  

4. **Construct Event Management Systems** (Modules 10–12)  
   - Event handling, synchronization, and system coordination.  

5. **Manage Game Engine Resources** (Modules 13–15)  
   - Resource management, input handling, and scripting support.  

Each milestone requires:  
- Team meetings for planning and task assignment.  
- Team-based contributions to engine design, implementation, and testing.  
- Individual development of a **mini-game** showcasing milestone features.  
- A **reflection paper** explaining contributions and decision-making.  


## 🛠️ Tools & Resources
- **SDL 3** – Core rendering library  
- **Code Editors** – Visual Studio 2022, VS Code, Eclipse, or CLion/Rider  
- **CMake** – Cross-platform build system  
- **Git Repository** – Version control (e.g., NCSU GitHub)  
- **Documentation Tools** – Markdown or LaTeX (with screenshots for visual evidence)  
- **Assets** – Sample Game Asset Manager (with citations for external resources)  


## 🤝 Collaboration Process
Teamwork is central to this project. Each team should:  
- Establish **ground rules** for expectations and responsibilities.  
- Use a **shared repository** with version control.  
- Hold **two weekly meetings** (virtual or in-person) with agendas and notes.  
- Use **collaboration tools** (Discord, Slack, Zoom) for communication.  
- Ensure equitable workload distribution and maintain role documentation.  


## 📦 Deliverables & Assessment

### Each Milestone Submission Includes
**Team Deliverables**:
- Updated engine source code with documentation and CMake build scripts.  
- A working build that compiles on **Ubuntu 24.04 LTS** or **Windows with VS 2022**.  

**Individual Deliverables**:
- A small playable mini-game demonstrating milestone features.  
- A **1–2 page reflection paper** on contributions, rationale, challenges, and future improvements.  
- **Appendix of screenshots** (not included in page count).  

### Grading Criteria (125 Points Total per Milestone)
- **50 pts** – Individual reflection writeup  
- **40 pts** – Functionality and correctness of engine features  
- **35 pts** – Team collaboration and documentation completeness  

> **Note:**   
> - Since we are CSC 581 students we will need to complete all components, including optional ones (graded out of 125 pts).  


# 📜 Team Ground Rules

### 1. Communication
- Primary platform: **Discord**  
- Major updates posted in **General** channel  
- Notify team if unavailable  

### 2. Meetings
- **Two weekly meetings** (virtual or in-person)  
- Agendas prepared in advance, notes recorded  
- Attendance expected; notify team if absent  

### 3. Work Distribution
- Tasks assigned equitably based on skill and workload  
- Members must meet deadlines  
- Ask for help early if struggling  

### 4. Code & Repository Standards
- Commit all code with **clear messages**  
- Ensure code compiles and runs on required platforms  
- Follow consistent naming, formatting, and documentation standards  

### 5. Accountability
- Contribute to both **engine development** and **individual game**  
- Write reflective papers individually (collaboration on ideas encouraged)  
- Missing deadlines or poor contributions will be addressed directly  

### 6. Collaboration & Respect
- Treat team members with professionalism and respect  
- Discuss differences constructively with problem-solving focus  
- Final decisions made by **consensus**; escalate unresolved issues to instructor  

### 7. Quality Assurance
- Test all code before merging to main branch  
- Review each milestone as a team before submission  
- Include code comments and milestone documentation as part of deliverables  

### 8. Academic Integrity
- Follow **University’s Academic Integrity Policy**  
- No plagiarism or misuse of assets/libraries  
- Properly cite external resources


# Suggested File Structure
<img width="670" height="634" alt="image" src="https://github.com/user-attachments/assets/b71afe1c-f664-46f2-ba85-4d4122c4eafc" />


# Install & Run
So your typical workflow is just:
```bash
powershell -ExecutionPolicy Bypass -File .\SetupEngine.ps1
```
or, with a custom exe name:
```bash
powershell -ExecutionPolicy Bypass -File .\SetupEngine.ps1 -Target "SkellyDemo.exe"
```


**or**
If you want to just build and run the game (SDL already downloaded)
```bash
cmake -S . -B build
cmake --build build
.\build\bin\Debug\GameRunner.exe
```
