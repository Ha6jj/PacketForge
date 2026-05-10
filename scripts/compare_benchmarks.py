import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

def get_latest_benchmark_dir(base_dir='build/benchmark_results/'):
    if not os.path.exists(base_dir):
        return None
    
    subdirs = [d for d in os.listdir(base_dir) if os.path.isdir(os.path.join(base_dir, d))]
    
    if not subdirs:
        return None
    
    latest_subdir = sorted(subdirs)[-1]
    return os.path.join(base_dir, latest_subdir)

def main():
    benchmark_base_dir = 'build/benchmark_results/'

    # 1. Получаем путь и сразу проверяем на существование
    latest_dir = get_latest_benchmark_dir(benchmark_base_dir)
    
    if not latest_dir:
        print(f"Ошибка: Не найдено папок с результатами в '{benchmark_base_dir}'")
        return

    print(f"Используем последние результаты из: {latest_dir}")

    latest_dir_name = os.path.basename(latest_dir)
    result_dir = os.path.join('build', 'comparison', latest_dir_name)
    os.makedirs(result_dir, exist_ok=True)

    pf_file = os.path.join(latest_dir, 'packetforge_benchmark_results.csv')
    pb_file = os.path.join(latest_dir, 'protobuf_benchmark_results.csv')

    if not all(os.path.exists(f) for f in [pf_file, pb_file]):
        print("Ошибка: Файлы packetforge_benchmark_results.csv или protobuf_benchmark_results.csv не найдены в выбранной папке.")
        print(f"   Содержимое папки: {os.listdir(latest_dir)}")
        return

    pf_df = pd.read_csv(pf_file)
    pb_df = pd.read_csv(pb_file)
    df = pd.concat([pf_df, pb_df], ignore_index=True)

    comp = df.pivot_table(
        index=['label', 'operation', 'trial'],
        columns='serializer',
        values=['avg_real_time_us', 'avg_cpu_time_us', 'payload_size_bytes', 'iterations', 'total_real_time_s']
    )

    comp.columns = ['_'.join(c).strip() for c in comp.columns]
    comp = comp.reset_index()

    comp['time_ratio'] = comp['avg_real_time_us_packetforge'] / comp['avg_real_time_us_protobuf']
    comp['cpu_ratio']  = comp['avg_cpu_time_us_packetforge'] / comp['avg_cpu_time_us_protobuf']
    
    comp['throughput_pf'] = comp['iterations_packetforge'] / comp['total_real_time_s_packetforge']
    comp['throughput_pb'] = comp['iterations_protobuf'] / comp['total_real_time_s_protobuf']
    comp['throughput_ratio'] = comp['throughput_pf'] / comp['throughput_pb']

    def fmt_speedup(x):
        if pd.isna(x): return "N/A"
        return f"{x:.2f}x медленнее" if x > 1 else f"{1/x:.2f}x быстрее"

    comp['time_speedup'] = comp['time_ratio'].apply(fmt_speedup)
    comp['throughput_speedup'] = comp['throughput_ratio'].apply(
        lambda x: f"{x:.2f}x {'выше' if x > 1 else 'ниже'}" if not pd.isna(x) else "N/A"
    )

    cols = [
        'label', 'operation', 'trial',
        'avg_real_time_us_packetforge', 'avg_real_time_us_protobuf', 'time_speedup',
        'payload_size_bytes_packetforge', 'payload_size_bytes_protobuf',
        'throughput_pf', 'throughput_pb', 'throughput_speedup'
    ]
    summary = comp[cols]
    summary_path = os.path.join(result_dir, 'benchmark_comparison_summary.csv')
    summary.to_csv(summary_path, index=False)
    print(f"Таблица сравнения сохранена: {summary_path}")

    plot_df = df[['serializer', 'label', 'operation', 'trial', 'avg_real_time_us']].copy()    
    
    plot_title = f"Сравнение производительности\n(Данные из: {os.path.basename(latest_dir)})"

    g = sns.catplot(
        data=plot_df,
        x='trial', y='avg_real_time_us', hue='serializer',
        col='label', row='operation', kind='bar', errorbar=None,
        height=3.5, aspect=1.2, palette='Set2'
    )
    
    for ax in g.axes.flat:
        ax.set_yscale('log')
        ax.tick_params(axis='x', rotation=45)
        
    g.fig.suptitle(plot_title, y=1.02)
    g.set_axis_labels('Конфигурация теста (trial)', 'Среднее время (мкс, лог. шкала)')
    g.tight_layout()
    
    graph_path = os.path.join(result_dir, 'benchmark_comparison.png')
    g.savefig(graph_path, dpi=300, bbox_inches='tight')
    print(f"График сохранён: {graph_path}")
    
    plt.show()

if __name__ == '__main__':
    main()