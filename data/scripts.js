const MAX_LINHAS = 50; // Limite de histórico no navegador
const TAXA_ATUALIZACAO_MS = 1000; // Taxa de atualização em milissegundos 

// --- Variáveis Globais para o Gráfico ---
let graficoValores;
const historicoTempo = [];
const historicoCidade = [];
const historicoBateria = [];

// --- Inicialização do Gráfico ---
function inicializarGrafico() {
  const ctx = document.getElementById('grafico_valores');
  if (!ctx) return;

  // Configuração global de estilos para o tema escuro
  Chart.defaults.color = '#94a3b8';
  Chart.defaults.font.family = 'system-ui, -apple-system, sans-serif';

  graficoValores = new Chart(ctx, {
    type: 'line',
    data: {
      labels: historicoTempo,
      datasets: [
        { label: 'Cidade (mW)', data: historicoCidade, borderColor: '#3b82f6', tension: 0.3, pointRadius: 0, borderWidth: 2 },
        { label: 'Bateria (mW)', data: historicoBateria, borderColor: '#10b981', tension: 0.3, pointRadius: 0, borderWidth: 2 }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      animation: false, // Mantido sem animação para otimizar a performance a cada 1s
      scales: {
        x: { grid: { color: '#334155' } },
        y: { 
          grid: { color: '#334155' },
          beginAtZero: true 
        }
      },
      plugins: {
        legend: { labels: { color: '#e2e8f0', usePointStyle: true } }
      }
    }
  });
}

// --- Funções de Manipulação de Dados ---
function atualizarGrafico(pCidade, pBateria) {
  if (!graficoValores) return;

  const agora = new Date();
  const hora = `${agora.getHours().toString().padStart(2, '0')}:${agora.getMinutes().toString().padStart(2, '0')}:${agora.getSeconds().toString().padStart(2, '0')}`;

  // Adiciona apenas os dados necessários ao histórico
  historicoTempo.push(hora);
  historicoCidade.push(pCidade);
  historicoBateria.push(pBateria);

  // Remove dados antigos para evitar sobrecarga de memória
  if (historicoTempo.length > MAX_LINHAS) {
    historicoTempo.shift();
    historicoCidade.shift();
    historicoBateria.shift();
  }

  graficoValores.update();
}

function adicionarLinha(idTabela, v, c, p) {
  const tbody = document.querySelector(`#${idTabela} tbody`);
  if (!tbody) return; 

  const novaLinha = document.createElement('tr');
  novaLinha.innerHTML = `
    <td>${v}</td>
    <td>${c}</td>
    <td>${p}</td>
  `;
  
  tbody.prepend(novaLinha);

  if (tbody.children.length > MAX_LINHAS) {
    tbody.removeChild(tbody.lastElementChild);
  }

  novaLinha.classList.add('linha-nova');
}

// --- Loop Principal ---
function atualizar() {
  fetch('/dados')
    .then(res => res.json())
    .then(data => {
      // Atualiza apenas as tabelas da Cidade (mini1) e da Bateria (master)
      adicionarLinha('tabela_cidade', data.v_mini1.toFixed(2), data.c_mini1.toFixed(2), data.p_mini1.toFixed(2));
      adicionarLinha('tabela_bateria', data.v_master.toFixed(2), data.c_master.toFixed(2), data.p_master.toFixed(2));

      // Atualiza o gráfico com as potências correspondentes
      atualizarGrafico(data.p_mini1, data.p_master);

      if (data.estado) {
        const estadoTxt = document.getElementById('estado_txt');
        estadoTxt.innerText = data.estado;

        // Altera a cor consoante o estado para dar feedback visual imediato
        switch (data.estado) {
          case "APAGAO": estadoTxt.style.color = "#ef4444"; break; // Vermelho
          case "NOITE": estadoTxt.style.color = "#3b82f6"; break; // Azul
          case "MANHA": estadoTxt.style.color = "#eab308"; break; // Amarelo
          case "TARDE": estadoTxt.style.color = "#f97316"; break; // Laranja
          default: estadoTxt.style.color = "#10b981"; break; // Verde (padrão)
        }
      }
    })
    .catch(err => console.error("Erro ao obter dados da telemetria:", err));
}

// Inicialização do fluxo
inicializarGrafico();
setInterval(atualizar, TAXA_ATUALIZACAO_MS);