#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

sem_t* tobacco_and_matches_sem;
sem_t* paper_and_tobacco_sem;
sem_t* paper_and_matches_sem;
sem_t* put_sem;

int HAS_TOBACCO = 0;
int HAS_PAPER = 1;
int HAS_MATCHES = 2;

int total_smokes = 0;
int smoke_seconds = 1;
char* finished_smoking = "has finished smoking";
char* is_smoking = "and is smoking";

void* smoker(void* args) {
    int type = *((int*) args);
    sem_t* sem;
    char* name;
    char* got_components;
    if (type == HAS_TOBACCO) {
        sem = paper_and_matches_sem;
        name = "Smoker with tobacco";
        got_components = "got paper and matches";
    } else if (type == HAS_PAPER) {
        sem = tobacco_and_matches_sem;
        name = "Smoker with paper";
        got_components = "got tobacco and matches";
    } else if (type == HAS_MATCHES) {
        sem = paper_and_tobacco_sem;
        name = "Smoker with matches";
        got_components = "got paper and tobacco";
    }
    while (total_smokes < 10) {
        if (sem_wait(sem) != 0 || total_smokes >= 10) {
            continue;
        }
        // здесь курят все по-очереди
        printf("%s %s %s\n", name, got_components, is_smoking);
        // будем на 1 секунду засыпать, чтобы показать процесс крурения
        sleep(smoke_seconds);
        printf("%s %s\n", name, finished_smoking);
        sem_post(put_sem);
    }
    return NULL;
}

void* middleman(void* args) {
    for (; total_smokes < 10; ++total_smokes) {
        if (sem_wait(put_sem) != 0) {
            continue;
        }
        switch (rand() % 3) {
            case 0:
                sem_post(paper_and_tobacco_sem);
                break;
            case 1:
                sem_post(paper_and_matches_sem);
                break;
            case 2:
                sem_post(tobacco_and_matches_sem);
                break;
        }
    }
    sem_post(paper_and_tobacco_sem);
    sem_post(paper_and_matches_sem);
    sem_post(tobacco_and_matches_sem);
    return NULL;
}

int main() {
    srand(time(NULL));
    if ((put_sem = sem_open("put_sem", O_CREAT | O_EXCL, 0660, 1)) == SEM_FAILED) {
        sem_unlink("put_sem");
        put_sem = sem_open("put_sem", O_CREAT | O_EXCL, 0660, 1);
    }
    if ((paper_and_tobacco_sem = sem_open("paper_and_tobacco_sem", O_CREAT | O_EXCL, 0660, 0)) == SEM_FAILED) {
        sem_unlink("paper_and_tobacco_sem");
        paper_and_tobacco_sem = sem_open("paper_and_tobacco_sem", O_CREAT | O_EXCL, 0660, 0);
    }
    if ((paper_and_matches_sem = sem_open("paper_and_matches_sem", O_CREAT | O_EXCL, 0660, 0)) == SEM_FAILED) {
        sem_unlink("paper_and_matches_sem");
        paper_and_matches_sem = sem_open("paper_and_matches_sem", O_CREAT | O_EXCL, 0660, 0);
    }
    if ((tobacco_and_matches_sem = sem_open("tobacco_and_matches_sem", O_CREAT | O_EXCL, 0660, 0)) == SEM_FAILED) {
        sem_unlink("tobacco_and_matches_sem");
        tobacco_and_matches_sem = sem_open("tobacco_and_matches_sem", O_CREAT | O_EXCL, 0660, 0);
    }

    pthread_t threads[4];
    pthread_create(threads + 0, NULL, middleman, NULL);
    pthread_create(threads + 1, NULL, smoker, &(HAS_MATCHES));
    pthread_create(threads + 2, NULL, smoker, &(HAS_PAPER));
    pthread_create(threads + 3, NULL, smoker, &(HAS_TOBACCO));

    for (int i = 0; i < 4; ++i) {
        pthread_join(threads[i], NULL);
    }

    sem_close(put_sem);
    sem_close(paper_and_tobacco_sem);
    sem_close(paper_and_matches_sem);
    sem_close(tobacco_and_matches_sem);
    return 0;
}
