# Mensageiro-Local

## Formatos de arquivos

### Mensagens de texto
Os dois usuários devem ser capazes de fazer envios de mensagem de texto com todos os caracteres disponíveis na codificação UTF-8. Isso inclui por exemplo letras acentuadas e emojis.

As mensagens devem ser enviadas utilizando para-e-espera, onde próxima mensagem só é enviada após a confirmação (ACK) da mensagem anterior.

### Arquivos de mídia
Os dois usuários devem ser capazes de fazer envios de qualquer tipo de arquivo de mídia, independente do formato e do tamanho do arquivo.

Visto que arquivos de mídia podem ter tamanhos arbitrários, o uso de janelas deslizantes é necessário independente da quantidade de arquivos enviados de uma só vez.

A janela deve ser volta-N, sendo um bônus implementa janela seletiva. O tamanho da janela é de 16.

## Implementação
De forma semelhante ao editor de texto modal vi ou de atalhos emacs, o envio de mensagens e mídias pode ser feito via comandos.
i: Inicia a criação de uma mensagem. Enter para enviar.
<esc>: Sai do modo de inserção de uma mensagem.
:q<enter>: Sai do programa.
:send x<enter>: Envia arquivo x.

 ## Formato da mensagem de comunicação
Inspirado no modelo Kermit e possui os seguintes campos, na ordem que eles são enviados na rede: 
  
  ![image](https://github.com/Rubens-Zan/Mensageiro-Local/assets/80857600/378749c4-3fba-4ab2-a14b-73c82e28f1e4)

  Marcador de início: A sequência de bits 01111110 (126 em decimal, ou 0x7e em hexadecimal).
Tipo: Pode ser um de
  texto: 0x01
  mídia: 0x10
  ack: 0x0A
  nack: 0x00
  erro: 0x1E
  inicio de transmissão: 0x1D
  fim de transmissão: 0x0F
  dados: 0x0D
Sequência: A sequência local da mensagem, devendo ser incrementada pela própria máquina ao enviar. Esta sequência varia de 0 a 15 de forma circular.
Tamanho: Tamanho do campo de dados, não incluindo nenhum outro campo do protocolo, variando de 0 a 63 bytes. Isso significa que o tamanho máximo de uma mensagem completa, com todos os campos, é de 63 (dados) + 3 (cabeçalho) + 1 (verificação) = 67 bytes.
Dados: Campo definido de acordo com o tipo da mensagem e de tamanho definido pelo campo anterior.
  texto: este campo codifica o texto da mensagem em UTF-8.
  mídia: este campo codifica o início do envio de um arquivo, e deve conter o tamanho do arquivo em bytes utilizando 4 bytes.
  ack: este campo codifica até qual sequência foi aceita (inclusive) em 1 byte.
  nack: este campo codifica qual sequência não foi recebida corretamente em 1 byte, e pede retransmissão da mesma.
  erro: este campo codifica um erro em 1 byte da enumeração:
  mídia rejeitada devido a espaço insuficiente para arquivo: 207
CRC-8: A soma de verificação da mensagem, que deve ser feita em relação ao campo de dados. Caso o tamanho do mesmo seja zero, esse campo ficará zerado. Deve ser utilizado uma verificação cíclica de redundância de 8 bits. Existem alguns polinômios disponíveis, e você pode escolher o que utilizar. O polinômio utilizado no 3G (WCDMA) por exemplo é o x8+x7+x4+x3+x+1 codificado no número 0x9B (0b10011011).
Utilizamos a codificação baseada em treliça para encode do conteúdo da mensagem e decode utilizando o método da distância de Hanning, para tratamento de erros em envio.  
