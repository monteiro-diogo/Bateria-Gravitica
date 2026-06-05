const MAX_LINHAS = 50; // Limite de histórico no navegador
const TAXA_ATUALIZACAO_MS = 1000; // Taxa de atualização em milissegundos 

function adicionarLinha(idTabela, v, c, p) {
  const tbody = document.querySelector(`#${idTabela} tbody`);
  
  // Evita erros se a tabela não for encontrada
  if (!tbody) return; 

  const novaLinha = document.createElement('tr');
  novaLinha.innerHTML = `
    <td>${v}</td>
    <td>${c}</td>
    <td>${p}</td>
  `;
  
  // Insere os dados mais recentes no topo da tabela
  tbody.prepend(novaLinha);

  // Remove a linha mais antiga se exceder o limite
  if (tbody.children.length > MAX_LINHAS) {
    tbody.removeChild(tbody.lastElementChild);
  }

  // Adiciona classe para animação de piscar
  novaLinha.classList.add('linha-nova');
}

function atualizar() {
  fetch('/dados')
    .then(res => res.json())
    .then(data => {
      adicionarLinha('tabela_rede', data.v_mini2.toFixed(2), data.c_mini2.toFixed(2), data.p_mini2.toFixed(2));
      adicionarLinha('tabela_cidade', data.v_mini1.toFixed(2), data.c_mini1.toFixed(2), data.p_mini1.toFixed(2));
      adicionarLinha('tabela_bateria', data.v_master.toFixed(2), data.c_master.toFixed(2), data.p_master.toFixed(2));

      if (data.estado) {
        document.getElementById('estado_txt').innerText = data.estado;
      }
    })
    .catch(err => console.error("Erro ao obter dados da telemetria:", err));
}

setInterval(atualizar, TAXA_ATUALIZACAO_MS);