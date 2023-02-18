Lib para funções do servidor: 
server_lib
server

Lib para funções do cliente: 
client_lib
client

Lib para correção de erros: 
error-handle
generate-message -> error-handle
list -> error-handle
binary-tree -> error-handle

Servidor e Cliente usam:
utils
generate-message -> error-handle
ConexaoRawSocket -> utils
