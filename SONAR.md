## SonarQube Analysis

To analyze the project with SonarQube, you need to have Docker and Docker Compose installed.

1.  **Start SonarQube:**
    From the `docker` directory, run:
    ```bash
    docker compose -f sonar.yml up -d
    ```
    Wait for SonarQube to be up and running. You can check the status at `http://localhost:9000`. The default credentials are `admin` / `admin`. You will be asked to change the password on the first login.

2.  **Generate a Token:**
    Log in to SonarQube and generate a new token by going to `My Account > Security`. Give the token a name and click `Generate`. Copy the token.

3.  **Run the Analysis:**
    From the project root, run the following command, replacing `<YOUR_SONAR_TOKEN>` with the token you generated:
    ```bash
    make sonar-scan token=<YOUR_SONAR_TOKEN>
    ```
    This command will compile the project with the build wrapper and then run the Sonar Scanner with your token.

    After the analysis is complete, the results will be available in your SonarQube dashboard at `http://localhost:9000`.

