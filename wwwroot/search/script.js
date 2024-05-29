const MAX_RESULTS_PER_PAGE_COUNT = 10;
const MAX_PAGES_LIST_SIZE = 4;
const API_URL = 'http://localhost:8080';

var query;
var total_size;
var current_size;
var current_page;

function handle_updated_query() {
    if (!query)
        query = localStorage.getItem('query');
    else {
        const cur_query = $('.user-input__query').first().val();
        if (cur_query === query || !cur_query)
            return;
        localStorage.setItem('query', cur_query);
        query = cur_query;
    }
    current_page = 1;
    fetch_data(current_page, function(results) {
        total_size = results.total_size;
        current_size = results.size;
        update_pagination();
        render_results(results);
    });
}

function fetch_data(page, callback) {
    fetch(
        `${API_URL}/search?query=${query}&limit=${MAX_RESULTS_PER_PAGE_COUNT}&offset=${(page - 1) * MAX_RESULTS_PER_PAGE_COUNT}`, {
        method: 'GET',
        mode: 'cors'
    })
    .then(response => response.json())
    .then(result => callback(result));
}

function update_results() {
    fetch_data(current_page, function(results) {
        current_size = results.size;
        render_results(results);
    });
}

function update_pagination() {
    let list = $('.page-buttons-container').first();
    list.empty();
    const size = current_page * MAX_RESULTS_PER_PAGE_COUNT;
    const prev_size = size - MAX_RESULTS_PER_PAGE_COUNT;
    show_page_move_button($('.page-list-switcher-prev').first(), size > MAX_RESULTS_PER_PAGE_COUNT);
    show_page_move_button($('.page-list-switcher-next').first(), size < total_size);
    if (!total_size)
        return;
    const more_pages = 
        Math.floor((total_size - prev_size) / MAX_RESULTS_PER_PAGE_COUNT) + 
        ((total_size - prev_size) % MAX_RESULTS_PER_PAGE_COUNT > 0);
    console.log(more_pages);
    for(let i = 0; i < Math.min(more_pages, MAX_PAGES_LIST_SIZE); ++i) {
        let button = generate_page_button(current_page + i);
        button.addEventListener('click', function () {
            const num = Number(button.textContent);
            current_page = num;
            $('.page-buttons-container > .page-button').each(function(ind, btn) {
                set_page_button_focus($(btn), false);
            });
            set_page_button_focus($(button), true);
            update_results();
            window.scrollTo(0, 0);
        });
        list.append(button);
    }
    set_page_button_focus(list.children().first(), true);
    window.scrollTo(0, 0);
}

function set_page_button_focus(btn, focused) {
    btn.css('background-color', focused ? '#1dc411' : '#20b2aa');
}

function show_page_move_button(btn, show) {
    btn.css('display', show ? 'block' : 'none');
}

function render_results(results) {
    let list = $('.search-results__list').first();
    list.empty();
    results.results.forEach(el => {
        const gen = generate_one_result(el);
        list.append(gen);
    });
}

function generate_page_button(number) {
    return from_html(`
        <button class="page-button">${number}</button>
    `);
}

function generate_one_result(data) {
    return from_html(`
        <li class="search-results__item">
            <div class="item-info">
                <a href="${data.url}"><h2 class="item-title">${data.title.trim()}</h2></a>
                <a class="item-url" href="${data.url}">${data.url}</a>
            </div>
            <div class="item-relevance">
                <img class="item-relevance__img" src="../assets/page_rank.svg" alt="ranking">
                <div class="item-relevance__desc">
                    ${create_relevance_caption(data.relevance)} %
                </div>
            </div>
        </li>
    `);
}

function create_relevance_caption(value) {
    const percentage = Math.round(value * 100);
    return (percentage === 0)
        ? '> 1'
        : `${percentage}`;
}

function from_html(html, trim = true) {
    html = trim ? html.trim() : html;
    if (!html) 
        return null;
    const template = document.createElement('template');
    template.innerHTML = html;
    const result = template.content.children;
    return (result.length === 1) 
        ? result[0] 
        : result;
}

function handle_next() {
    const remain = current_page % MAX_PAGES_LIST_SIZE;
    const diff = (remain) ? (MAX_PAGES_LIST_SIZE - remain + 1) : 1;
    current_page += diff;
    update_pagination();
    update_results();
}

function handle_previous() {
    const remain = current_page % MAX_PAGES_LIST_SIZE;
    const diff = (remain) ? remain + MAX_PAGES_LIST_SIZE - 1 : 2 * MAX_PAGES_LIST_SIZE - 1;
    current_page -= diff;
    update_pagination();
    update_results();
}

$(function() {
    $(this).keypress(function (ev) {
        var keycode = (ev.keyCode ? ev.keyCode : ev.which);
        if (keycode == '13') {
            handle_updated_query();
        }
    });
    $('.user-input__icon').first().bind('click', function(e) {
        handle_updated_query();
    });
    $('.page-list-switcher-prev').first().bind('click', function(e) {
        handle_previous();
    });
    $('.page-list-switcher-next').first().bind('click', function(e) {
        handle_next();
    });    
    handle_updated_query();
});