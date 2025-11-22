#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>

#define RX_PIN 13  // D7 (GPIO13)
#define TX_PIN 15  // D8 (GPIO15)

SoftwareSerial mySerial(RX_PIN, TX_PIN);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


struct Digital {
    int id;
    String nome;
};


Digital digitais[100];
int nextID = 1;
int totalDigitais = 0;

void setup() {
    Serial.begin(115200);
    mySerial.begin(57600);

    Serial.println("\nInicializando sensor de impressão digital...");

    if (finger.verifyPassword()) {
        Serial.println("Sensor encontrado!");
    } else {
        Serial.println("Sensor não encontrado. Verifique as conexões.");
        while (1);
    }
}

void loop() {
    Serial.println("\nEscolha uma opção:");
    Serial.println("1 - Cadastrar digital");
    Serial.println("2 - Verificar digital");
    Serial.println("3 - Mostrar informações da digital");
    Serial.println("4 - Limpar todas as digitais");

    String input = readUserInput();
    char opcao = input.charAt(0);
    
    Serial.println();

    if (opcao == '1') {
        cadastrarDigital();
    } else if (opcao == '2') {
        verificarDigital();
    } else if (opcao == '3') {
        mostrarInformacoes();
    } else if (opcao == '4') {
        limparDigitais();
    } else {
        Serial.println("Opção inválida! Tente novamente.");
    }
}

String readUserInput() {
    String input = "";
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n') {
                break;
            }
            input += c;
        }
    }
    return input;
}

void cadastrarDigital() {
    int id = nextID;
    
    Serial.print("\nVerificando se a digital já está cadastrada...");
    if (finger.loadModel(id) == FINGERPRINT_OK) {
        Serial.println("\nEsta digital já está cadastrada!");
        return;
    }

    // Pergunta o nome do portador
    Serial.println("\nDigite o nome do portador da digital:");
    String nome = readUserInput();

    Serial.println("\nColoque seu dedo no sensor...");
    while (finger.getImage() != FINGERPRINT_OK);

    if (finger.image2Tz(1) != FINGERPRINT_OK) {
        Serial.println("Erro ao converter imagem!");
        return;
    }

    Serial.println("Remova o dedo...");
    delay(2000);

    Serial.println("Coloque o mesmo dedo novamente...");
    while (finger.getImage() != FINGERPRINT_OK);

    if (finger.image2Tz(2) != FINGERPRINT_OK) {
        Serial.println("Erro ao converter imagem!");
        return;
    }

    if (finger.createModel() == FINGERPRINT_OK) {
        Serial.println("Impressão confirmada! Salvando...");
        if (finger.storeModel(id) == FINGERPRINT_OK) {
            Serial.println("Impressão digital salva com sucesso!");

            
            digitais[totalDigitais].id = id;
            digitais[totalDigitais].nome = nome;
            totalDigitais++;

            nextID++; 
        } else {
            Serial.println("Erro ao salvar impressão.");
        }
    } else {
        Serial.println("As duas leituras não coincidem.");
    }
}

void verificarDigital() {
    Serial.println("\nAproxime seu dedo para verificação...");
    Serial.println("Digite 0 a qualquer momento para cancelar.");

    while (true) {
        
        if (Serial.available()) {
            char input = Serial.read();
            if (input == '0') {
                Serial.println("Operação cancelada pelo usuário.");
                return;
            }
        }

        
        int result = finger.getImage();

        if (result == FINGERPRINT_OK) {
            Serial.println("Imagem capturada com sucesso!");
            break;
        } else if (result == FINGERPRINT_NOFINGER) {
            delay(500);
        } else {
            Serial.print("Erro ao capturar imagem: ");
            Serial.println(result);
            return;
        }
    }

    int result = finger.image2Tz(1);
    if (result != FINGERPRINT_OK) {
        Serial.print("Erro ao converter imagem: ");
        Serial.println(result);
        return;
    }

    result = finger.fingerFastSearch();
    if (result == FINGERPRINT_OK) {
        String nomePortador = "Desconhecido";
        for (int i = 0; i < totalDigitais; i++) {
            if (digitais[i].id == finger.fingerID) {
                nomePortador = digitais[i].nome;
                break;
            }
        }

        Serial.print("Digital reconhecida! ID: ");
        Serial.print(finger.fingerID);
        Serial.print(", Nome: ");
        Serial.print(nomePortador);
        Serial.print(", Pontuação: ");
        Serial.println(finger.confidence);
    } else {
        Serial.print("Digital não encontrada no banco de dados. Erro: ");
        Serial.println(result);
    }
}

void mostrarInformacoes() {
    Serial.println("\nDigite o ID da digital que deseja consultar:");
    String input = readUserInput();
    int id = input.toInt();
    
    Serial.print("\nVerificando informações do ID ");
    Serial.println(id);

    bool encontrada = false;
    for (int i = 0; i < totalDigitais; i++) {
        if (digitais[i].id == id) {
            Serial.println("Digital encontrada!");
            Serial.print("ID da digital: ");
            Serial.println(digitais[i].id);
            Serial.print("Nome do portador: ");
            Serial.println(digitais[i].nome);
            encontrada = true;
            break;
        }
    }

    if (!encontrada) {
        Serial.println("Nenhuma digital cadastrada nesse ID.");
    }
}

void limparDigitais() {
    Serial.println("\nTem certeza que deseja apagar todas as digitais? (s/n)");
    String input = readUserInput();
    char confirmacao = input.charAt(0);

    if (confirmacao == 's' || confirmacao == 'S') {
        int result = finger.emptyDatabase();
        if (result == FINGERPRINT_OK) {
            Serial.println("Todas as digitais foram apagadas com sucesso!");
            nextID = 1;
            totalDigitais = 0;
        } else {
            Serial.print("Erro ao apagar digitais: ");
            Serial.println(result);
        }
    } else {
        Serial.println("Operação cancelada.");
    }
}
