
## Arquitetura do Sistema
```
[main()]
  |
  |-- Inicializa stack TCP/IP via EthernetInterface
  |-- Instancia:
  |     - TCPCtrlIntfModule  → Interface de controle via socket TCP
  |     - CtrlCoreModule     → Processamento dos comandos SCPI
  |
  |-- Inicia as threads dos dois módulos
  |
  |-- Loop para monitoramento da rede
```

---

### **TCPCtrlIntfModule**

- Receber comandos SCPI do cliente TCP
- Enviar comandos à fila para o `CtrlCoreModule`
- Aguardar processamento e retornar resposta via socket

**Implementação**:
- Contém um `CtrlIntfModuleMessage`, que encapsula:
  - Um buffer de dados `char[]`
  - Um `Semaphore` de sincronização
- Quando um comando SCPI é recebido:
  - É armazenado em `buff`
  - O `CtrlIntfModuleMessage` com o buffer + semáforo é enfileirado via `_try_put_for_cb`
  - A thread bloqueia no `ready.acquire()`, aguardando o processamento
  - Após `release()` pelo `CtrlCoreModule`, a resposta é enviada ao cliente via `client->send`

---

### **CtrlCoreModule**

- Extrair comandos da fila
- Encaminhar para o interpretador SCPI
- Sinalizar o semáforo após processar

**Implementação**:
- Recebe callback `try_get_for_cb` para ler da fila
- Possui:
  - `SCPIIntModule` → interpretador SCPI
  - `GlobalRFFE` → interface de controle do hardware
- A cada ciclo do `_task()`:
  - Lê `CtrlIntfModuleMessage` da fila
  - Passa `buff` para `SCPIIntModule::Process()`
  - Após o comando ser processado, chama `p_ready->release()` para liberar o TCP

---

### **SCPIIntModule**

- Interpretar e executar comandos SCPI
- Enviar respostas ou erros via `SCPI_Write` / `SCPI_Error`

**Implementação**:
- Registrado com `SCPI_Init()` com os callbacks:
  - `.write` → `SCPIIntModule::SCPI_Write`
  - `.error` → `SCPIIntModule::SCPI_Error`
- `user_context = this` → permite acessar membros da classe
- `Process(cmd)` chama `SCPI_Input()` e armazena resposta no `_response_buffer`
- `SCPI_Write()` copia os dados da resposta para `_response_buffer`

---

###  **Resumo do Fluxo**

```

[CLIENTE TCP]
  |
  |-- Envia comando SCPI via socket TCP
  |
  |-- [TCPCtrlIntfModule]
  |     |-- Recebe comando do cliente
  |     |-- Enfileira mensagem (buff + semáforo) para CtrlCoreModule
  |     |-- Aguarda processamento (ready.acquire)
  |     |-- Após processamento, envia resposta via socket
  |
  |-- [CtrlCoreModule]
  |     |-- Extrai mensagem da fila
  |     |-- Chama _scpi_mod.Process(cmd)
  |
  |-- [SCPIIntModule]
        |-- Executa comando SCPI
        |-- Gera resposta via função .write()
        |-- Finaliza comando e libera semáforo (p_ready->release)

```

